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


#define BUF_SIZE 0x10
#define MAX_DEVICES 1

#define SEM710_BAUDRATE 19200

typedef enum { 
  WAITING_FOR_START, 
  WAITING_FOR_REPLY_START, 
  WAITING_FOR_COMMAND, 
  WAITING_FOR_LEN, 
  WAITING_FOR_DATA, 
  WAITING_FOR_CRC_LO, 
  WAITING_FOR_CRC_HI, 
  WAITING_FOR_END,
  RX_OVERFLOW
} RX_FRAMING;

typedef enum {
  WAITING,
  GET_FUNCTION,
  GET_CODE,
  start_address,
  NO_BYTES,
  GET_BYTES,
  GET_CRC_LOW,
  GET_CRC_HIGH,
  ENGAGED,
  MESSAGE_NOT_VALID,
} COMMS_RX_STATE;

typedef struct {
  float ADC_VALUE;
  float ELEC_VALUE;
  float PROCESS_VARIALBE;
  float MA_OUT;
  float CJ_TEMP;
} SEM710_READINGS;

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

int open_device(FT_HANDLE *ftHandle, int vendor_id, int product_id) {

  // now use FTD2XX to open the device
  FT_STATUS ftStatus;
  FT_DEVICE_LIST_INFO_NODE *pDest = NULL;
  DWORD dwNumDevs;

  ftStatus = FT_SetVIDPID((DWORD) vendor_id, (DWORD) product_id);
  if (ftStatus != FT_OK) {
    printf("Problem setting vID and pID\n");
    return -1;
  }

  ftStatus = FT_CreateDeviceInfoList(&dwNumDevs);
  if (ftStatus != FT_OK) {
    printf("Error: FT_CreateDeviceInfoList(%d)\n", (int) ftStatus);
    return -1;
  }

  ftStatus = FT_GetDeviceInfoList(pDest, &dwNumDevs);
  if (ftStatus != FT_OK) {
    printf("Error: FT_ListDevices(%d)\n", (int) ftStatus);
    return -1;
  }

  ftStatus = FT_Open(0, ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error: FT_Open(%d)\n", ftStatus);
    return -1;
  }

  // open successful
  return 0;
}


int prepare_device(FT_HANDLE ftHandle) 
{
  // prepare the device for communication, per the specifications provided by
  // Status Instruments about the attributes of the SEM710
  FT_STATUS ftStatus;
  ftStatus = FT_ResetDevice(ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error: FT_ResetDevice(%d)\n", ftStatus);
    return -1;
  }

  ftStatus = FT_SetBaudRate(ftHandle, SEM710_BAUDRATE);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetBaudRate(%d)\n", ftStatus);
    return -1;
  }

  ftStatus = FT_SetDataCharacteristics(ftHandle, 8, 0, 0);
  /*
    FT_DATA_BITS_8 == 8
    FT_STOP_BITS_1 == 0
    FT_PARITY_NONE == 0
  */
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetDataCharacteristics(%d)\n", ftStatus);
    return -1;
  }

  ftStatus = FT_SetFlowControl(ftHandle, 0x00, 0, 0);
  // FT_FLOW_NONE == &H0 == 0x00
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetFlowControl(%d)\n", ftStatus);
    return -1;
  }

  ftStatus = FT_SetTimeouts(ftHandle, 250, 250);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetTimeouts(%d)\n", ftStatus);
    return -1;
  }

  ftStatus = FT_SetLatencyTimer(ftHandle, 3);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetLatencyTimer(%d)\n", ftStatus);
    return -1;
  }
  return 0;
}



uint16_t make_crc(uint8_t *byte_array, int end_position)
{
  int i;
  uint16_t CRC;
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

  CRC = 0xFFFF;
  for (i = 1; i <= end_position; i++) {
    char_in = byte_array[i];
    CRC = CRC ^ char_in;
    for (j = 0; j < 8; j++) {
      lsBit = CRC & 1;
      CRC = CRC >> 1;
      if (lsBit == 1) {
	CRC = CRC ^ 0xA001;
      }
    }
  }
  return CRC;
}

int crc_pass(uint8_t *rx_data, DWORD rx_pointer) 
{
  uint16_t calculated_crc = make_crc(rx_data, rx_pointer - 2);
  uint16_t rx_crc = (rx_data[rx_pointer] << 8) + (rx_data[rx_pointer - 1]);

  return (calculated_crc == rx_crc);
}

void generate_read_message(uint8_t byte_array[6])
{
  // Fill the given byte array with bytes corresponding to instructions to the
  // SEM710 to read its current temperature measurement

  uint16_t crc;
  uint16_t lbyte;
  uint16_t rbyte;

  // Start Byte:
  byte_array[0] = 0x55;
  
  // Command (cREAD_PROCESS):
  byte_array[1] = 0x02; 

  // number of elements in byte_array at the time of input; according to the
  // specifications provided by Status Instruments, this byte_array should have
  // one 0 byte in its first index value, so fill the array accordingly.
  byte_array[2] = 0;
  byte_array[3] = 0;

  // cyclic redundancy check:
  crc = make_crc(byte_array, 3);
  lbyte = (crc >> 8);
  rbyte = (crc) & 0xFF;

  // put CRC on byte_array, little Endian
  byte_array[4] = rbyte;
  byte_array[5] = lbyte;

  // End byte
  byte_array[6] = 0xAA;
}

int send_and_receive_bytes(uint8_t* byte_array, int byte_array_size, 
			   FT_HANDLE ftHandle, uint8_t* output_array)
{
  // uint8_t timeout;
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
    ftStatus = FT_Write(ftHandle, byte_array, byte_array_size, &bytes_written);
    if (ftStatus != FT_OK) {
      printf("Error FT_Write(%d)\n.", ftStatus);
      return 1;
    }
    printf("Bytes written: %d\n", bytes_written);
    usleep(200000); // 200 ms
    while ((difftime(expire, start) < 2.5) && (!message_received)) {
      ftStatus = FT_GetQueueStatus(ftHandle, &q_status);
      if (ftStatus != FT_OK) {
	printf("Error FT_GetQueueStatus(%d)\n", ftStatus);
      }
      if (q_status != 0) { // then there is something to read
	ftStatus = FT_Read(ftHandle, read_buff, bytes_to_read, &q_status);
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
      ftStatus = FT_Purge(ftHandle, FT_PURGE_TX & FT_PURGE_RX);
    }
  }
  return (!message_received);
}

float float_from_byte_array(uint8_t *byte_array, int start_index)
{
  // bytes are stored little Endian, so reverse first
  int i;
  float f1;
  uint8_t chars[4];

  for (i = 0; i < 4; i++) {
    chars[3-i] = byte_array[start_index + i];
  }
  f1 = *((float *)(&chars[0]));
  return f1;
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

  //////////////////////////
  // 1: Connect to server //
  //////////////////////////

  // unimplemented stub //

  ///////////////////////////
  // 2: Open SEM710 device //
  ///////////////////////////
  int detach_failed;
  int open_failed;
  int prep_failed;
  FT_HANDLE ftHandle;
  
  detach_failed = detach_device_kernel(vendor_id, product_id);
  if (detach_failed) { return 1; }
  
  open_failed = open_device(&ftHandle, vendor_id, product_id);
  if (open_failed) { return 1; }

  prep_failed = prepare_device(ftHandle);
  if (prep_failed) { 
    FT_Close(ftHandle);
    return 1; 
  }

  // 2.5: report open status
  
  //////////////////////////////////////////
  // Communication to server unimplemented//
  //////////////////////////////////////////

  printf("Device prepared.\n");

  // 3: await instructions

  // 4: read data from SEM710
  int i;
  uint8_t byte_array[6];
  uint8_t *response_array;
  int rw_failed;
  FT_STATUS ftStatus;
  // long intArray[3];

  generate_read_message(byte_array);
  printf("Message: [");
  printf("%u,", byte_array[0]);
  for (i = 1; i < 6; i++) {
    printf(" %u,", byte_array[i]);
  }
  printf(" %u]\n", byte_array[6]);

  rw_failed = send_and_receive_bytes(byte_array, 7, ftHandle, response_array);
  if (rw_failed || response_array[1] != 34) {
    printf("Problem reading or writing data to device.\n");
    FT_Close(ftHandle);
    return 1;
  }

  // decode reply
  SEM710_READINGS readings;
  readings.ADC_VALUE = float_from_byte_array(response_array, 3);
  readings.ELEC_VALUE = float_from_byte_array(response_array, 7);
  readings.PROCESS_VARIALBE = float_from_byte_array(response_array, 11);
  readings.MA_OUT = float_from_byte_array(response_array, 15);
  readings.CJ_TEMP = float_from_byte_array(response_array, 19);
  
  // 4.5: transmit data from SEM710

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
  
  return 0;
}
