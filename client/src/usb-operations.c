#ifndef _USBOP
#define _USBOP

#include "../ftd2xx/ftd2xx.h"
#include <libusb-1.0/libusb.h>
#include "../devtypes.h"
#include "array.c"

uint16_t make_crc(uint8_t *byte_array, int end_position)
{
  // make a cyclic redundancy check for the given byte array; this is per the
  // rules found in the SEM710 specifications provided by Status Instruments; 
  // the polynomial 0xA001 doesn't appear to conform to any international
  // crc standard.
  int i;
  uint16_t crc;
  char j;
  char char_in;
  int lsBit;

  /*
    The CRC is based on the following components of this message:
    
    Start
    Command
    Length
    Data
  */

  crc = 0xFFFF;
  for (i = 1; i <= end_position; i++) {
    char_in = byte_array[i];
    crc = crc ^ char_in;
    for (j = 0; j < 8; j++) {
      lsBit = crc & 1;
      crc = crc >> 1;
      if (lsBit == 1) {
	crc = crc ^ 0xA001;
      }
    }
  }
  return crc;
}

int crc_pass(uint8_t *rx_data, DWORD rx_pointer) 
{
  // returns whether the received crc is the same as the calculated crc from
  // the byte array passed by the USB device; if it is not, then the usb 
  // transfer was faulty
  uint16_t calculated_crc = make_crc(rx_data, rx_pointer - 2);
  uint16_t rx_crc = (rx_data[rx_pointer] << 8) + (rx_data[rx_pointer - 1]);

  return (calculated_crc == rx_crc);
}

int detach_device_kernel(int vendor_id, int product_id) 
{
  // if the device kernel with the given vendor id and product id is currently 
  // active then detach it and return whether or not this action failed. This 
  // must be done using libusb, since ftd2xx does not have this capability.

  int detach_failed;
  libusb_context *context = NULL;
  libusb_init(&context);
  libusb_set_debug(context, 3);
  libusb_device_handle *handle = NULL;

  handle = libusb_open_device_with_vid_pid(context, vendor_id, product_id);
  if (handle == NULL) {
    printf("Error opening device: device missing.\n");    
    return -1;
  }
  
  if (libusb_kernel_driver_active(handle, 0)) {
    printf("Kernel driver already attached. Detaching...\n");
    detach_failed = libusb_detach_kernel_driver(handle, 0);
    if (detach_failed) {
      printf("There was an error detaching the kernel driver.\n");
      return -1;
    }
  }
  libusb_release_interface(handle, 0);
  libusb_close(handle);
  libusb_exit(context);
  
  return 0;
}

FT_STATUS open_device(FT_HANDLE *ftHandle, int vendor_id, int product_id) 
{
  // use FTD2XX to open the device; this open device function simply opens the
  // first device it finds with the given vendor and product id, and cannot
  // differentiate between multiple devices with the same vID and pID
  FT_STATUS ftStatus;
  DWORD dwNumDevs;

  ftStatus = FT_SetVIDPID((DWORD) vendor_id, (DWORD) product_id);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetVIDPID(%d)\n", (int) ftStatus);
    return ftStatus;
  }

  ftStatus = FT_CreateDeviceInfoList(&dwNumDevs);
  if (ftStatus != FT_OK) {
    printf("Error: FT_CreateDeviceInfoList(%d)\n", (int) ftStatus);
    return ftStatus;
  }

  ftStatus = FT_Open(0, ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error: FT_Open(%d)\n", (int) ftStatus);
    return ftStatus;
  }

  // open successful
  return FT_OK;
}


FT_STATUS prepare_device(FT_HANDLE *ftHandle) 
{
  // prepare the device for communication, per the specifications provided by
  // Status Instruments about the attributes of the SEM710
  FT_STATUS ftStatus;
  ftStatus = FT_ResetDevice(*ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error: FT_ResetDevice(%d)\n", ftStatus);
    return ftStatus;
  }

  ftStatus = FT_SetBaudRate(*ftHandle, SEM710_BAUDRATE);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetBaudRate(%d)\n", ftStatus);
    return ftStatus;
  }

  ftStatus = FT_SetDataCharacteristics(*ftHandle, FT_DATA_BITS_8, 
				       FT_STOP_BITS_1, FT_PARITY_NONE);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetDataCharacteristics(%d)\n", ftStatus);
    return ftStatus;
  }

  ftStatus = FT_SetFlowControl(*ftHandle, FT_FLOW_NONE, 0, 0);
  // FT_FLOW_NONE == &H0 == 0x00
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetFlowControl(%d)\n", ftStatus);
    return ftStatus;
  }

  ftStatus = FT_SetTimeouts(*ftHandle, 250, 250);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetTimeouts(%d)\n", ftStatus);
    return ftStatus;
  }

  ftStatus = FT_SetLatencyTimer(*ftHandle, 3);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetLatencyTimer(%d)\n", ftStatus);
    return ftStatus;
  }
  return FT_OK;
}

int generate_message(uint8_t COMMAND, uint8_t* byte_array,
		     int byte_array_upper_index) 
{
  // Generate a message to send to the device, based on the given command and
  // byte array; returns the size of the message
  uint8_t output[259];
  int i = 0;
  uint8_t x;
  uint16_t crc;
  uint8_t lbyte;
  uint8_t rbyte;

  // start byte
  output[i] = 0x55;

  // command
  output[++i] = COMMAND;

  // length
  output[++i] = byte_array_upper_index;
  for (x = 0; x <= byte_array_upper_index; x++) {
    output[++i] = byte_array[x];
  }

  // crc
  crc = make_crc(output, i);
  lbyte = (crc >> 8);
  rbyte = (crc) & 0xFF;

  // put CRC on byte_array, little Endian
  output[++i] = rbyte;
  output[++i] = lbyte;

  // add end byte;
  output[++i] = 0xAA;
  for (x = 0; x <= i; x++) {
    byte_array[x] = output[x];
  }

  return (i + 1);
}

int generate_block_message(uint8_t device_address, int command, 
			   uint8_t byte_count, long block_address, 
			   uint8_t *byte_array, uint8_t read)
{
  // generate a block message and return the length of said block message
  uint8_t output[280];

  int i = 0;
  uint8_t x = 0;

  uint16_t crc;
  uint8_t lbyte;
  uint8_t rbyte;

  long temp;

  // data enters in the byte_array
  output[0] = device_address;
  output[1] = command;
  
  output[2] = byte_count;
  // start address Msb then lsb
  temp = block_address & 0xFF00;
  output[4] = block_address - (temp);
  output[3] = temp / (2 << 8);

  i = 4;
  if (!read) {
    for (x = 0; x < byte_count; x++) {
      i++;
      output[i] = byte_array[x];
    }
  }
  
  crc = make_crc(output, i);
  lbyte = (crc >> 8);
  rbyte = (crc) & 0xFF;

  // put CRC on byte_array, little Endian
  output[++i] = rbyte;
  output[++i] = lbyte;

  for (x = 0; x <= i; x++) {
    byte_array[x] = output[x];
  }

  return i + 1;
}

int send_bytes(uint8_t* byte_array, int byte_array_size, 
	       FT_HANDLE *ftHandle, uint8_t* output_array)
{
  // transmit the given byte array to the USB device and await a reply
  DWORD q_status;
  uint8_t read_buff[280];
  uint8_t retry_counter = 4;
  DWORD i;
  int count;
  uint8_t rx_byte;
  int rx_len;
  uint8_t message_received;
  RX_FRAMING rx_frame;
  uint8_t rx_data[280];
  DWORD rx_pointer;
  time_t start;
  time_t expire;
  // uint8_t byte;
  FT_STATUS ftStatus;
  DWORD bytes_to_read = 262;
  DWORD bytes_written;
  
  while (retry_counter > 0) {
    usleep(50000); // 50 ms
    time(&start);
    time(&expire);
    message_received = 0;
    q_status = 0;
    rx_frame = WAITING_FOR_START;

    ftStatus = FT_Write(*ftHandle, byte_array, byte_array_size, &bytes_written);
    if (ftStatus != FT_OK) {
      printf("Error FT_Write(%d)\n.", ftStatus);
      return 1;
    }
    printf("Writing %d bytes... ", bytes_written);
    usleep(200000); // 200 ms
    while ((difftime(expire, start) < 2.5) && (!message_received)) {
      ftStatus = FT_GetQueueStatus(*ftHandle, &q_status);
      if (ftStatus != FT_OK) {
	printf("Error FT_GetQueueStatus(%d)\n", ftStatus);
      }
      if (q_status != 0) { // then there is something to read
	ftStatus = FT_Read(*ftHandle, read_buff, bytes_to_read, &q_status);
	usleep(50000); // 50 ms
	for (i = 0; i <= q_status; i++) {
	  rx_byte = read_buff[i];
	  if (rx_frame == WAITING_FOR_START) {
	    if (rx_byte == 0x55) {
	      rx_pointer = 0;
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	      rx_frame = WAITING_FOR_COMMAND;
	      count = 0;
	      rx_len = 0;
	    }
	  }
	  else if (rx_frame == WAITING_FOR_REPLY_START) {
	    if (rx_byte == 0x55) {
	      rx_pointer = 0;
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	      rx_frame = WAITING_FOR_COMMAND;
	      count = 0;
	      rx_len = 0;
	    }
	  }
	  else if ((rx_frame == WAITING_FOR_COMMAND) ||
	      (rx_frame == WAITING_FOR_LEN)) 
	    {
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	      if (rx_frame == WAITING_FOR_LEN) {
		rx_len = rx_byte - 1;
		rx_frame = WAITING_FOR_DATA;
	      }
	      else {
		rx_frame = WAITING_FOR_LEN;
	      }
	    }
	  else if (rx_frame == WAITING_FOR_DATA) {
	    if (count < rx_len) {
	      count = count + 1;
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	    }
	    else {
	      // this is CRC low
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	      rx_frame = WAITING_FOR_CRC_HI;
	    }
	  }
	  else if (rx_frame == WAITING_FOR_CRC_LO) {
	    rx_data[rx_pointer] = rx_byte;
	    rx_pointer = rx_pointer + 1;
	    rx_frame = WAITING_FOR_CRC_HI;
	  }
	  else if (rx_frame == WAITING_FOR_CRC_HI) {
	    rx_data[rx_pointer] = rx_byte;
	    rx_pointer = rx_pointer + 1;
	    rx_frame = WAITING_FOR_END;
	  }
	  else if (rx_frame == WAITING_FOR_END) {
	    if (rx_byte == 0xAA) {
	      rx_frame = RX_OVERFLOW;
	      if (crc_pass(rx_data, rx_pointer) == 0) {
	        message_received = 0;
	      }
	      else {
	        for (i = 0; i <= rx_pointer; i++) {
		  output_array[i] = rx_data[i];
		}
		message_received = 1;
	      }
	    }
	  }
	  else if (rx_frame == RX_OVERFLOW) {
	    break;
	  }
	} // end for (q_status condition)
      } // end if (q_status condition)
      time(&expire);
    } // end while (time expire condition)
    if (!message_received) {
      printf("no reply. Retrying.\n");
      retry_counter = retry_counter - 1;
      ftStatus = FT_Purge(*ftHandle, FT_PURGE_TX & FT_PURGE_RX);
    }
  } // end while (retry condition)
  return (!message_received);
}

int send_block_message(uint8_t *byte_array, int byte_array_size, 
		       uint8_t device_address, FT_HANDLE *ftHandle,
		       uint8_t *output_array)
{
  time_t start;
  time_t expire;

  DWORD q_status;
  uint8_t read_buff[280];
  uint8_t retry_counter = 4;

  uint8_t i, j;
  int count;
  uint8_t rx_byte;
  uint8_t byte_count;
  uint8_t message_received;
  COMMS_RX_STATE rx_frame;
  uint8_t rx_data[280];
  int rx_pointer = 0;
  FT_STATUS ftStatus;
  DWORD bytes_written;
  DWORD bytes_to_read = 262;
  
  while (retry_counter) {
    usleep(50000); // 50 ms
    time(&start);
    time(&expire);
    message_received = 0;
    q_status = 0;
    rx_frame = WAITING;

    ftStatus = FT_Write(*ftHandle, byte_array, byte_array_size, &bytes_written);
    if (ftStatus != FT_OK) {
      printf("Error FT_Write(%d)\n.", ftStatus);
      return 1;
    }
    printf("Bytes written: %d\n", bytes_written);
    usleep(200000); // 200 ms
    while ((difftime(expire, start) < 2.5) && (!message_received)) {
      ftStatus = FT_GetQueueStatus(*ftHandle, &q_status);
      //      printf(
      if (ftStatus != FT_OK) {
	printf("Error FT_GetQueueStatus(%d)\n", ftStatus);
	return -1;
      }
      if (q_status != 0) { // then there is something to read
	ftStatus = FT_Read(*ftHandle, read_buff, bytes_to_read, &q_status);
	if (ftStatus != FT_OK) {
	  printf("Error FT_Read(%d)\n", ftStatus);
	  return -1;
	}
	usleep(50000); // 50 ms
	for (i = 0; i <= q_status; i++) {
	  rx_byte = read_buff[i];
	  if (rx_frame == WAITING) {
	    if (rx_byte == device_address) {
	      rx_pointer = 0;
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	      rx_frame = GET_FUNCTION;
	      count = 0;
	    }
	  }
	  else if (rx_frame == GET_FUNCTION) {
	    rx_data[rx_pointer] = rx_byte;
	    rx_pointer = rx_pointer + 1;
	    if (rx_byte > 127) {
	      rx_frame = GET_CODE;
	    }
	    else {
	      rx_frame = NO_BYTES;
	    }
	  }
	  else if (rx_frame == GET_CODE) {
	    rx_data[rx_pointer] = rx_byte;
	    rx_pointer = rx_pointer + 1;
	    rx_frame = GET_CRC_LOW;
	  }
	  else if (rx_frame == NO_BYTES) {
	    byte_count = rx_byte;
	    rx_data[rx_pointer] = rx_byte;
	    rx_pointer = rx_pointer + 1;
	    rx_frame = GET_BYTES;
	  }
	  else if (rx_frame == GET_BYTES) {
	    if (count < byte_count) {
	      count = count + 1;
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	    }
	    else {
	      rx_data[rx_pointer] = rx_byte;
	      rx_pointer = rx_pointer + 1;
	      rx_frame = GET_CRC_HIGH;
	    }
	  }
	  else if (rx_frame == GET_CRC_LOW) {
	    rx_data[rx_pointer] = rx_byte;
	    rx_pointer = rx_pointer + 1;
	    rx_frame = GET_CRC_HIGH;
	  }
	  else if (rx_frame == GET_CRC_HIGH) {
	    rx_data[rx_pointer] = rx_byte;
	    rx_pointer = rx_pointer + 1;
	    rx_frame = ENGAGED;
	    if (!crc_pass(rx_data, rx_pointer)) {
	      printf("CRC failed. Message did not transmit properly.\n");
	      return -1;
	    }
	    else {
	      for (j = 0; j < rx_pointer; j++) {
		output_array[j] = rx_data[j];
	      }
	      message_received = 1;
	    }
	  }
	} // end for (q_status condition)
      } // end if(q_status condition)
      time(&expire);
    } // end while(time condition)
    if (!message_received) {
      retry_counter = retry_counter - 1;
      ftStatus = FT_Purge(*ftHandle, FT_PURGE_TX & FT_PURGE_RX);
    }
  } // end while(retry condition)
  return (!message_received);
}

int SEM710_read_process(FT_HANDLE *ftHandle, SEM710_READINGS *readings)
{
  int i;
  uint8_t byte_array[256];
  int len;
  uint8_t response_array[256];
  int rw_failed;

  byte_array[0] = 0;
  len = generate_message(SEM_COMMANDS_cREAD_PROCESS, byte_array, 0);
  printf("Message: [%u,", byte_array[0]);
  for (i = 1; i < len - 1; i++) {
    printf(" %u,", byte_array[i]);
  }
  printf(" %u]\n", byte_array[len - 1]);

  rw_failed = send_bytes(byte_array, len, ftHandle, response_array);
  if (rw_failed || response_array[1] != 34) {
    printf("Problem reading or writing data to device.\n");
    return -1;
  }
  // decode reply
  readings->ADC_VALUE = float_from_byte_array(response_array, 3);
  readings->ELEC_VALUE = float_from_byte_array(response_array, 7);
  readings->PROCESS_VARIABLE = float_from_byte_array(response_array, 11);
  readings->MA_OUT = float_from_byte_array(response_array, 15);
  readings->CJ_TEMP = float_from_byte_array(response_array, 19);

  return 0;
}

int SEM710_read_config_block(FT_HANDLE *ftHandle, CONFIG_BLOCK *cal, 
			     uint8_t device_address)
{
  uint8_t byte_array[280];
  uint8_t output_array[280];
  byte_array[0] = 0;
  int i;
  int len;
  int err;
  
  len = generate_block_message(device_address, COMMAND_FUNCTION_READ_DATA,
			       0xEC, 0xE000, byte_array, 1);
  printf("Message: [%u,", byte_array[0]);
  for (i = 1; i < len - 1; i++) {
    printf(" %u,", byte_array[i]);
  }
  printf(" %u]\n", byte_array[len - 1]);

  err = send_block_message(byte_array, len, device_address, ftHandle, 
			     output_array);
  if (err) {
    printf("Error sending block message\n");
    return -1;
  } 

  // Decode reply:
  CONFIG_BLOCK_init(cal);
  for (i = 0; i < 48; i++) {
    cal->fp[i] = float_from_byte_array(output_array, (i * 4) + 3);
  }
  for (i = 0; i < 4; i++) {
    cal->config_input_float[i] = float_from_byte_array(output_array, 195+(i*4));
  }
  for (i = 0; i < 4; i++) {
    cal->config_input_byte[i] = short_from_byte_array(output_array, 211+(i*2));
  }
  string_from_byte_array(output_array, cal->title, 219, 234);
  string_from_byte_array(output_array, cal->units, 235, 238);

  // send another message :S
  byte_array[0] = 0;
  len = generate_block_message(device_address, COMMAND_FUNCTION_READ_DATA, 64,
			       4096, byte_array, 1);
  if (len <= 0) {
    printf("Error generating message\n");
    return -1;
  }
  err = send_block_message(byte_array, len, device_address, ftHandle, 
			     output_array);
  if (err) {
    printf("Error sending block message\n");
    return -1;
  }
  if (output_array[1] != 0x04) {
    printf("Error sending block message: bad reply\n");
    return -1;
  }

  // decode second reply:
  for (i = 0; i < 8; i++) {
    cal->config_output_floats[i] = float_from_byte_array(output_array, (i*4)+3);
  }
  for (i = 0; i < 4; i++) {
    cal->config_output_int[i] =short_from_byte_array(output_array, 35+(i*2));
  }
  string_from_byte_array(output_array, cal->tag_number, 43, 62);

  return 0;
}

int SEM710_read_config(FT_HANDLE *ftHandle, CONFIG_DATA *cal)
{
  uint8_t byte_array[256];
  int len = -1;
  long i;
  byte_array[0] = 0;
  uint8_t response_array[256];
  int rw_failed;

  len = generate_message(SEM_COMMANDS_cREAD_CONFIG, byte_array, 0);
  if (len == -1) {
    printf("Failed to generate message.\n");
    return -1;
  }
  printf("Message: [%u,", byte_array[0]);
  for (i = 1; i < len - 1; i++) {
    printf(" %u,", byte_array[i]);
  }
  printf(" %u]\n", byte_array[len - 1]);

  rw_failed = send_bytes(byte_array, len, ftHandle, response_array);
  if (rw_failed || (response_array[1] != 34)) {
    printf("Problem reading or writing data to device.\n");
    return -1;
  }
  
  cal->tc_code = response_array[3];
  cal->up_scale = response_array[4];
  cal->units = response_array[5];
  cal->model_type = response_array[6];
  cal->vout_range = response_array[7];
  cal->action_A = response_array[8];
  cal->action_B = response_array[9];
  cal->spare = response_array[10];
  cal->low_range = float_from_byte_array(response_array, 11);
  cal->high_range = float_from_byte_array(response_array, 15);
  cal->low_trim = float_from_byte_array(response_array, 19);
  cal->high_trim = float_from_byte_array(response_array, 23);
  cal->setpoint_A = float_from_byte_array(response_array, 27);
  cal->hyst_A = float_from_byte_array(response_array, 31);
  cal->setpoint_B = float_from_byte_array(response_array, 35);
  cal->hyst_B = float_from_byte_array(response_array, 39);

  return 0;
}

int SEM710_write_config(FT_HANDLE *ftHandle, CONFIG_DATA *cal)
{
  uint8_t byte_array[41];
  int i;
  int len = -1;
  uint8_t output_array[280];
  int err;
  
  byte_array[0] = cal->tc_code;
  byte_array[1] = cal->up_scale;
  byte_array[2] = cal->units;
  byte_array[3] = cal->model_type;
  byte_array[4] = cal->vout_range;
  byte_array[5] = cal->action_A;
  byte_array[6] = cal->action_B;
  byte_array[7] = cal->spare;
  i = float_into_byte_array(byte_array, 8, cal->low_range);
  i = float_into_byte_array(byte_array, i, cal->high_range);
  i = float_into_byte_array(byte_array, i, cal->low_trim);
  i = float_into_byte_array(byte_array, i, cal->high_trim);
  i = float_into_byte_array(byte_array, i, cal->setpoint_A);
  i = float_into_byte_array(byte_array, i, cal->hyst_A);
  i = float_into_byte_array(byte_array, i, cal->setpoint_B);
  i = float_into_byte_array(byte_array, i, cal->hyst_B);  

  len = generate_message(SEM_COMMANDS_cSET_CONFIG, byte_array, i-1);
  if (len <= 0) {
    printf("Error generating message");
    return -1;
  }

  printf("Message: [%u,", byte_array[0]);
  for (i = 1; i < len - 1; i++) {
    printf(" %u,", byte_array[i]);
  }
  printf(" %u]\n", byte_array[len - 1]);
  
  err = send_bytes(byte_array, len, ftHandle, output_array);
  if (err || output_array[1] != 10) {
    return -1;
  }

  printf("Config written successfully.\n");
  return 0;
}

int SEM710_read_cal(FT_HANDLE *ftHandle, UNIVERSAL_CALIBRATION *cal)
{
  uint8_t byte_array[280];
  uint8_t output_array[280];
  byte_array[0] = 0;
  int len = -1;
  int err;
  
  len = generate_message(SEM_COMMANDS_cREAD_CAL, byte_array, 0);
  if (len <= 0) {
    printf("Error generating message.\n");
    return -1;
  }

  err = send_bytes(byte_array, len, ftHandle, output_array);
  if (err || output_array[1] != 32) {
    printf("Error sending bytes to device\n");
    return -1;
  }

  // decode reply
  cal->lo_mv = float_from_byte_array(output_array, 5);
  cal->hi_mv = float_from_byte_array(output_array, 9);
  cal->lo_ma = float_from_byte_array(output_array, 13);
  cal->hi_ma = float_from_byte_array(output_array, 17);
  cal->lo_rtd = float_from_byte_array(output_array, 21);
  cal->hi_rtd = float_from_byte_array(output_array, 25);
  cal->lo_ma_in = float_from_byte_array(output_array, 29);
  cal->hi_ma_in = float_from_byte_array(output_array, 33);

  return 0;
}

int SEM710_set_cal(FT_HANDLE *ftHandle, UNIVERSAL_CALIBRATION *cal)
{
  uint8_t byte_array[280];
  uint8_t output_array[280];
  int err;
  int len = -1;
  int i = 0;
  
  byte_array[i] = 0;
  byte_array[++i] = 0;
  i = float_into_byte_array(byte_array, i, cal->lo_mv);
  i = float_into_byte_array(byte_array, i, cal->hi_mv);
  i = float_into_byte_array(byte_array, i, cal->lo_ma);
  i = float_into_byte_array(byte_array, i, cal->hi_ma);
  i = float_into_byte_array(byte_array, i, cal->lo_rtd);
  i = float_into_byte_array(byte_array, i, cal->hi_rtd);
  i = float_into_byte_array(byte_array, i, cal->lo_ma_in);
  i = float_into_byte_array(byte_array, i, cal->hi_ma_in);

  len = generate_message(SEM_COMMANDS_cSET_CAL, byte_array, i - 1);
  if (len <= 0) {
    printf("Error generating message\n");
    return -1;
  }

  err = send_bytes(byte_array, len, ftHandle, output_array);
  if (err || (output_array[1] != 10)) {
    printf("Error sending bytes\n");
    return -1;
  }
  
  return 0;
}

int SEM710_enable_retran(FT_HANDLE *ftHandle)
{
  uint8_t byte_array[280];
  uint8_t output_array[280];
  int len = -1;
  int err;

  byte_array[0] = 0;
  
  len = generate_message(SEM_COMMANDS_cPRESET_ENABLE, byte_array, 0);
  if (len <= 0) {
    printf("Error generating message\n");
    return -1;
  }
  
  err = send_bytes(byte_array, len, ftHandle, output_array);
  if (err) {
    printf("Error sending bytes\n");
    return -1;
  }

  return 0;
}

int SEM710_self_cal(FT_HANDLE *ftHandle, int command)
{
  /*
    cal commands are:
    SEM_COMMANDS_cSELF_CAL_100R
    SEM_COMMANDS_cSELF_CAL_300R
    SEM_COMMANDS_cSELF_CAL_0mV
    SEM_COMMANDS_cSELF_CAL_50mv
    SEM_COMMANDS_cSELF_CAL_0mA
    SEM_COMMANDS_cSELF_CAL_20mA
   */
  uint8_t byte_array[280];
  uint8_t output_array[280];
  int len = -1;
  int err;

  byte_array[0] = 0;
  
  len = generate_message(command, byte_array, 0);
  if (len <= 0) {
    printf("Error generating message\n");
    return -1;
  }
  
  err = send_bytes(byte_array, len, ftHandle, output_array);
  if (err) {
    printf("Error sending bytes\n");
    return -1;
  }

  return 0;
}

#endif
