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
  float PROCESS_VARIABLE;
  float MA_OUT;
  float CJ_TEMP;
} SEM710_READINGS;

typedef struct {
  uint8_t cACK;
  uint8_t cNAK;
  uint8_t cREAD_CAL;                    
  uint8_t cREAD_CONFIG;
  uint8_t cREAD_PROCESS;                 
  uint8_t cSELF_CAL_0mv;                
  uint8_t cSELF_CAL_50mv;               
  uint8_t cSELF_CAL_100R;              
  uint8_t cSELF_CAL_300R;              
  uint8_t cSELF_CAL_20mA;              
  uint8_t cSELF_CAL_0mA;               
  uint8_t cSELF_CAL_200mV;             
  uint8_t cSELF_CAL_1V;                
  uint8_t cSELF_CAL_10V;               
  uint8_t cSELF_CAL_slide_wire;        
  uint8_t cPRESET_4ma_COUNT;            
  uint8_t cPRESET_12ma_COUNT;           
  uint8_t cPRESET_20ma_COUNT;           
  uint8_t cPRESET_ENABLE;               
  uint8_t cSET_CAL;                    
  uint8_t cSET_CONFIG;                 
  uint8_t cREAD_RANGEA;
  uint8_t cREAD_RANGEB;
  uint8_t cREAD_RANGEC;
  uint8_t cREAD_RANGED;
  uint8_t cWRITE_RANGEA;
  uint8_t cWRITE_RANGEB;
  uint8_t cWRITE_RANGEC;
  uint8_t cWRITE_RANGED;
  uint8_t cidentify;

  uint8_t cREAD_SEM160_CAL;
  uint8_t cREAD_SEM160_CONFIG;
  uint8_t cREAD_SEM160_ALIGNMENT;
  uint8_t cREAD_SEM160_PROCESS;
  uint8_t cREAD_SEM160_DEVICE;
  uint8_t cPRESET_ch2_4ma_COUNT;
  uint8_t cPRESET_ch2_12ma_COUNT;
  uint8_t cPRESET_ch2_20ma_COUNT;
  uint8_t cPRESET_ch2_ENABLE;

  uint8_t cALIGN_RH_1;
  uint8_t cALIGN_RH_2;
  uint8_t cALIGN_T_1;
  uint8_t cALIGN_T_2;

  uint8_t cWRITE_SEM160_CALIBRATION;
  uint8_t cWRITE_SEM160_CONFIG;
  uint8_t cWRITE_SEM160_ALIGNMENT;
  uint8_t cWRITE_SEM160_DEVICE;

  uint8_t cWRITE_TTR_4_5MA_CAL;
  uint8_t cWRITE_TTR_19_5MA_CAL;
  uint8_t cWRITE_TTR_1_5V_CAL;
  uint8_t cWRITE_TTR_9_5V_CAL;
  uint8_t cWRITE_TTx200_IN_GAIN_1;
  uint8_t cWRITE_TTx200_IN_GAIN_2;
  uint8_t cWRITE_TTx200_IN_GAIN_4;

  uint8_t cWRITE_BLOCK_2_0_MA_CAL;
  uint8_t cWRITE_BLOCK_19_5_MA_CAL;
  uint8_t cWRITE_BLOCK_0_V_CAL;
  uint8_t cWRITE_BLOCK_10V_CAL;
  uint8_t cWRITE_BLOCK_CALIBRATION;
  uint8_t cREAD_BLOCK_CALIBRATION;
  uint8_t cWRITE_block_INPUT_CONFIG;
  uint8_t cWRITE_BLOCK_OUTPUT_CONFIG;
  uint8_t cREAD_BLOCK_INPUT_CONFIG;
  uint8_t cREAD_BLOCK_OUTPUT_CONFIG;
  uint8_t cREAD_BLOCK_PROCESS;

  uint8_t cWRITE_DM2000_CONFIG;
  uint8_t cREAD_DM2000_CONFIG;
  uint8_t cRESET_TOTAL;
} SEM_COMMANDS;

void SEM_COMMANDS_init(SEM_COMMANDS *sem_commands)
{
  sem_commands->cACK = 0xA;
  sem_commands->cNAK = 0xB;
  sem_commands->cREAD_CAL = 0x0;
  sem_commands->cREAD_CONFIG = 0x1;
  sem_commands->cREAD_PROCESS = 0x2;
  sem_commands->cSELF_CAL_0mv = 0x3;
  sem_commands->cSELF_CAL_50mv = 0x4;
  sem_commands->cSELF_CAL_100R = 0x30;
  sem_commands->cSELF_CAL_300R = 0x31;
  sem_commands->cSELF_CAL_20mA = 0x32;
  sem_commands->cSELF_CAL_0mA = 0x33;  
  sem_commands->cSELF_CAL_200mV = 0x34;
  sem_commands->cSELF_CAL_1V = 0x35;
  sem_commands->cSELF_CAL_10V = 0x36;
  sem_commands->cSELF_CAL_slide_wire = 0x37;
  sem_commands->cPRESET_4ma_COUNT = 0x5;
  sem_commands->cPRESET_12ma_COUNT = 0x6;
  sem_commands->cPRESET_20ma_COUNT = 0x7;
  sem_commands->cPRESET_ENABLE = 0x8;
  sem_commands->cSET_CAL = 0x10;
  sem_commands->cSET_CONFIG = 0x11;
  sem_commands->cREAD_RANGEA = 0x50;
  sem_commands->cREAD_RANGEB = 0x51;
  sem_commands->cREAD_RANGEC = 0x52;
  sem_commands->cREAD_RANGED = 0x53;
  sem_commands->cWRITE_RANGEA = 0x40;
  sem_commands->cWRITE_RANGEB = 0x41;
  sem_commands->cWRITE_RANGEC = 0x42;
  sem_commands->cWRITE_RANGED = 0x43;
  sem_commands->cidentify = 0x60;

  sem_commands->cREAD_SEM160_CAL = 0x60;
  sem_commands->cREAD_SEM160_CONFIG = 0x61;
  sem_commands->cREAD_SEM160_ALIGNMENT = 0x62;
  sem_commands->cREAD_SEM160_PROCESS = 0x63;
  sem_commands->cREAD_SEM160_DEVICE = 0x64;
  sem_commands->cPRESET_ch2_4ma_COUNT = 0x70;
  sem_commands->cPRESET_ch2_12ma_COUNT = 0x71;
  sem_commands->cPRESET_ch2_20ma_COUNT = 0x72;
  sem_commands->cPRESET_ch2_ENABLE = 0x73;

  sem_commands->cALIGN_RH_1 = 0x65;
  sem_commands->cALIGN_RH_2 = 0x66;
  sem_commands->cALIGN_T_1 = 0x67;
  sem_commands->cALIGN_T_2 = 0x68;

  sem_commands->cWRITE_SEM160_CALIBRATION = 0x6A;
  sem_commands->cWRITE_SEM160_CONFIG = 0x6B;
  sem_commands->cWRITE_SEM160_ALIGNMENT = 0x6C;
  sem_commands->cWRITE_SEM160_DEVICE = 0x6D;

  sem_commands->cWRITE_TTR_4_5MA_CAL = 0xA3;
  sem_commands->cWRITE_TTR_19_5MA_CAL = 0xA4;
  sem_commands->cWRITE_TTR_1_5V_CAL = 0xA5;
  sem_commands->cWRITE_TTR_9_5V_CAL = 0xA6;
  sem_commands->cWRITE_TTx200_IN_GAIN_1 = 0x30;
  sem_commands->cWRITE_TTx200_IN_GAIN_2 = 0x31;
  sem_commands->cWRITE_TTx200_IN_GAIN_4 = 0xA7;

  sem_commands->cWRITE_BLOCK_2_0_MA_CAL = 0xA3;
  sem_commands->cWRITE_BLOCK_19_5_MA_CAL = 0xA4;
  sem_commands->cWRITE_BLOCK_0_V_CAL = 0xA5;
  sem_commands->cWRITE_BLOCK_10V_CAL = 0xA6;
  sem_commands->cWRITE_BLOCK_CALIBRATION = 0xA0;
  sem_commands->cREAD_BLOCK_CALIBRATION = 0xA8;
  sem_commands->cWRITE_block_INPUT_CONFIG = 0xA1;
  sem_commands->cWRITE_BLOCK_OUTPUT_CONFIG = 0xA2;
  sem_commands->cREAD_BLOCK_INPUT_CONFIG = 0xA9;
  sem_commands->cREAD_BLOCK_OUTPUT_CONFIG = 0xAA;
  sem_commands->cREAD_BLOCK_PROCESS = 0x2;

  sem_commands->cWRITE_DM2000_CONFIG = 0x7B;

  sem_commands->cREAD_DM2000_CONFIG = 0x81;

  sem_commands->cRESET_TOTAL = 0xF0;
}

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

  // use FTD2XX to open the device; this open device function simply opens the
  // first device it finds with the given vendor and product id, and cannot
  // differentiate between them.
  FT_STATUS ftStatus;
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
    args following ftHandle:
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
  // the byte array passed by the USB device; if it is not, then something
  // caused the transfer to fail.
  uint16_t calculated_crc = make_crc(rx_data, rx_pointer - 2);
  uint16_t rx_crc = (rx_data[rx_pointer] << 8) + (rx_data[rx_pointer - 1]);

  return (calculated_crc == rx_crc);
}

int generate_message(uint8_t COMMAND, uint8_t* byte_array,
		     int byte_array_upper_index) 
{
  // Generate a message to send to the device, based on the given command and
  // byte array
  uint8_t *output;
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
  i = 2;
  for (x = 0; x <= byte_array_upper_index; x++) {
    i = i + 1;
    output[i] = byte_array[x];
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
  return 0;
}

int send_bytes(uint8_t* byte_array, int byte_array_size, 
	       FT_HANDLE ftHandle, uint8_t* output_array)
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

  //////////////////////////////////////////
  // 3: await instructions (skip for now) //
  //////////////////////////////////////////
  
  //////////////////////////////
  // 4: read data from SEM710 //
  //////////////////////////////

  int i;
  uint8_t* byte_array;
  uint8_t *response_array;
  int rw_failed;
  FT_STATUS ftStatus;
  SEM_COMMANDS sem_commands;
  SEM_COMMANDS_init(&sem_commands);
  // long intArray[3];

  byte_array[0] = 0;
  generate_message(sem_commands.cREAD_PROCESS, byte_array, 0);
  printf("Message: [%u,", byte_array[0]);
  for (i = 1; i < 6; i++) {
    printf(" %u,", byte_array[i]);
  }
  printf(" %u]\n", byte_array[6]);

  rw_failed = send_bytes(byte_array, 7, ftHandle, response_array);
  if (rw_failed || response_array[1] != 34) {
    printf("Problem reading or writing data to device.\n");
    FT_Close(ftHandle);
    return 1;
  }

  // decode reply
  SEM710_READINGS readings;
  readings.ADC_VALUE = float_from_byte_array(response_array, 3);
  readings.ELEC_VALUE = float_from_byte_array(response_array, 7);
  readings.PROCESS_VARIABLE = float_from_byte_array(response_array, 11);
  readings.MA_OUT = float_from_byte_array(response_array, 15);
  readings.CJ_TEMP = float_from_byte_array(response_array, 19);

  // 4.5: transmit data from SEM710

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
  
  return 0;
}
