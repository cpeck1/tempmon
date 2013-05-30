#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <math.h>
#include "./ftd2xx/ftd2xx.h"
#include "devstats.c"

#define BUF_SIZE 0x10
#define MAX_DEVICES 1

#define SEM710_BAUDRATE 19200

int detach_device_kernel(int vendor_id, int product_id) 
{
  // if the device kernel with the given vendor id and product id is currently 
  // active then detach it and return whether or not this action failed

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
  // use libUSB to detach the device from the kernel driver first, since 
  // FTDXX has no such functionality

  // now use FTDXX to open the device
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

uint16_t make_CRC(uint8_t *byte_array, int end_position)
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
  for (i = 1; i < end_position; i++) {
    char_in = byte_array[i];
    CRC = CRC ^ char_in;
    for (j = 1; j < 9; j++) {
      lsBit = CRC & 1;
      CRC = (int) (lsBit / 2);
      if (lsBit == 1) {
	CRC = CRC ^ 0xA001;
      }
    }
  }
  return CRC;
}

void generate_read_message(uint8_t byte_array[6])
{
  uint16_t crc;
  uint16_t lbyte;
  uint16_t rbyte;

  // Start Byte:
  byte_array[0] = 0x55;
  
  // Command (cREAD_PROCESS):
  byte_array[1] = 0x02; 

  // highest index of input byte array (at time of input), in this case 0
  byte_array[2] = 0;

  // cyclic redundancy check:
  crc = make_CRC(byte_array, 3);
  lbyte = (crc >> 8) & 0xff;
  rbyte = (crc) & 0xff;

  // put CRC on byte_array, little Endian
  printf("CRC=%d\n", crc);
  printf("rbyte=%d\n", rbyte);
  printf("lbyte=%d\n", lbyte);

  byte_array[3] = rbyte;
  byte_array[4] = lbyte;

  // End byte
  byte_array[5] = 0xAA;
}

int send_bytes(uint8_t* byte_array)
{
  
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
  if (prep_failed) { return 1; }

  // 2.5: report open status
  
  //////////////////////////////////////////
  // Communication to server unimplemented//
  //////////////////////////////////////////

  printf("Device prepared.\n");

  // 3: await instructions

  // 4: read data from SEM710
  int i;
  uint8_t byte_array[6];
  // long intArray[3];

  generate_read_message(byte_array);
  for (i = 0; i < 6; i++) {
    printf("%u\n", byte_array[i]);
  }

  send_bytes(byte_array);
  
  // 4.5: transmit data from SEM710

  // 5: purge buffer; await further instructions

  return 0;
}
