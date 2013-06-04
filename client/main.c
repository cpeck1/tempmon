#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <math.h>
#include <time.h>
#include "./ftd2xx/ftd2xx.h"
#include "devstats.c"
#include "devtypes.h"

#define BUF_SIZE 0x10
#define MAX_DEVICES 1

#define L_ENDIAN (get_endianness() == 0)
#define B_ENDIAN (get_endianness() == 1)

#define SEM710_BAUDRATE 19200

int detach_device_kernel(int vendor_id, int product_id) 
{
  // if the device kernel with the given vendor id and product id is currently 
  // active then detach it and return whether or not this action failed. This 
  // must be done using libusb, since ftd2xx does not have this capability.

  int failed = 0;
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
    failed = libusb_detach_kernel_driver(handle, 0);
    if (failed) {
      printf("There was an error detaching the kernel driver.\n");
      return -1;
    }
  }
  libusb_release_interface(handle, 0);
  libusb_close(handle);
  libusb_exit(context);
  
  return 0;
}

FT_STATUS open_device(FT_HANDLE *ftHandle, int vendor_id, int product_id) {

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

  ftStatus = FT_SetDataCharacteristics(*ftHandle, 8, 0, 0);
  /*
    args following ftHandle:
    FT_DATA_BITS_8 == 8
    FT_STOP_BITS_1 == 0
    FT_PARITY_NONE == 0
  */
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetDataCharacteristics(%d)\n", ftStatus);
    return ftStatus;
  }

  ftStatus = FT_SetFlowControl(*ftHandle, 0x00, 0, 0);
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

    // implement:
    ftStatus = FT_Write(*ftHandle, byte_array, byte_array_size, &bytes_written);
    if (ftStatus != FT_OK) {
      printf("Error FT_Write(%d)\n.", ftStatus);
      return 1;
    }
    printf("Bytes written: %d\n", bytes_written);
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
	}
      }
      time(&expire);
    }
    if (!message_received) {
      retry_counter = retry_counter - 1;
      ftStatus = FT_Purge(*ftHandle, FT_PURGE_TX & FT_PURGE_RX);
    }
  }
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

    // implement:
    ftStatus = FT_Write(*ftHandle, byte_array, byte_array_size, &bytes_written);
    if (ftStatus != FT_OK) {
      printf("Error FT_Write(%d)\n.", ftStatus);
      return 1;
    }
    printf("Bytes written: %d\n", bytes_written);
    usleep(200000); // 200 ms
    while ((difftime(expire, start) < 2.5) && (!message_received)) {
      ftStatus = FT_GetQueueStatus(*ftHandle, &q_status);
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

float float_from_byte_array(uint8_t *byte_array, int start_index)
{
  // bytes are stored little Endian, so reverse first
  int i;
  float f1;
  int size = (int) sizeof(float);
  uint8_t chars[size];

  if (B_ENDIAN) {
    for (i = 0; i < size; i++) {
      chars[(size-1) - i] = byte_array[start_index + i];
    }
    f1 = *((float *)(&chars[0]));
  }
  else {
    f1 = *((float *)(&byte_array[start_index]));
  }
  return f1;
}

uint16_t short_from_byte_array(uint8_t *byte_array, int start_index)
{
  int i;
  short s1;
  int size = (int) sizeof(short);
  uint8_t chars[size];

  // byte_array = {14, 23} 
  // chars[1] = start_index[0]; chars[0] = start_index[1]
  // chars == [23, 14]
  
  if (B_ENDIAN) {
    for (i = 0; i < size; i++) {
      chars[(size-1) - i] = byte_array[start_index + i];
    }
    s1 = *((short *)(&chars[0]));
  }
  else {
    s1 = *((short *)(&byte_array[start_index]));
  }
  return s1;
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
  int error;
  
  len = generate_block_message(device_address, COMMAND_FUNCTION_READ_DATA,
			       0xEC, 0xE000, byte_array, 1);
  error = send_block_message(byte_array, len, device_address, ftHandle, 
			     output_array);
  if (error) {
    printf("Error sending block message\n");
    return -1;
  } 

  CONFIG_BLOCK_init(cal);

  for (i = 0; i < 48; i++) {
    cal->fp[i] = float_from_byte_array(output_array, (i * 4) + 3);
  }
  for (i = 0; i < 4; i++) {
    cal->config_input_float[i] = float_from_byte_array(output_array, 195+(i*4));
  }
  for (i = 0; i < 4; i++) {
  }

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

int main()
{
  // Get device ids from file:
  int product_id;
  int vendor_id;

  int err = 0;

  err = get_device_ids(&product_id, &vendor_id);
  if (err) {
    printf("Error: bad or missing device file\n");
    return 1;
  }
  
  uint8_t test[2] = {23, 14};
  // 14 = 00001110 == [14, 23] = 0000111000010111
  // 23 = 00010111 == [23, 14] = 0001011100001110
  short tests = short_from_byte_array(test, 0);
  printf("test[0]:%d\n", test[0]);
  printf("test[1]:%d\n", test[1]);
  printf("test:%d\n", tests);
  
  //////////////////////////
  // 1: Connect to server //
  //////////////////////////

  // unimplemented stub //

  ///////////////////////////
  // 2: Open SEM710 device //
  ///////////////////////////
  int detach_failed;
  FT_STATUS open_attempt;
  FT_STATUS prep_attempt;
  FT_HANDLE ftHandle;
  
  detach_failed = detach_device_kernel(vendor_id, product_id);
  if (detach_failed) { return 1; }
  
  open_attempt = open_device(&ftHandle, vendor_id, product_id);
  if (open_attempt) { return 1; }

  prep_attempt = prepare_device(&ftHandle);
  if (prep_attempt) { 
    FT_Close(ftHandle);
    return 1; 
  }

  // 2.5: report open status
  
  //////////////////////////////////////////
  // Communication to server unimplemented//
  //////////////////////////////////////////
  printf("Device prepared.\n");

  //////////////////////////////////////////
  // 3: await instructions (skip for now) //
  //////////////////////////////////////////
  
  //////////////////////////////
  // 4: read data from SEM710 //
  //////////////////////////////
  int read_failed;
  FT_STATUS ftStatus;
  SEM710_READINGS readings;
  CONFIG_BLOCK block;
  CONFIG_BLOCK_init(&block);
  CONFIG_DATA cal;

  read_failed = SEM710_read_config_block(&ftHandle, &block, 1);
  if (read_failed) {
    printf("Read config failure.\n");
    CONFIG_BLOCK_destroy(&block);
    FT_Close(ftHandle);
    return 1;
  }

  read_failed = SEM710_read_config(&ftHandle, &cal);

  read_failed = SEM710_read_process(&ftHandle, &readings);
  if (read_failed) {
    printf("Read process failure.\n");
    FT_Close(ftHandle);
    return 1;
  }

  // 4.5: transmit data from SEM710

  //////////////////////////////////////////
  // Communication to server unimplemented//
  //////////////////////////////////////////
  printf("Device ADC reading: %f\n", readings.ADC_VALUE);
  printf("Device ELEC reading: %f\n", readings.ELEC_VALUE);
  printf("Device PROCESS reading: %f\n", readings.PROCESS_VARIABLE);
  printf("Device MA reading: %f\n", readings.MA_OUT);
  printf("Device TEMP reading: %f\n", readings.CJ_TEMP);

  // 5: purge buffer; await further instructions

  // 6: close device
  ftStatus = FT_Close(ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error FT_Close(%d).\n", ftStatus);
    return 1;
  }
  else {
    printf("Device closed. Exiting...\n");
  }
  CONFIG_BLOCK_destroy(&block);
  
  return 0;
}
