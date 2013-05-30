#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "./ftd2xx/ftd2xx.h"
#include "devstats.c"

#define BUF_SIZE 0x10
#define MAX_DEVICES 1

#define SEM710_BAUDRATE 19200

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
  printf("Product id: %d Vendor id: %d\n", product_id, vendor_id);

  //////////////////////////
  // 1: Connect to server //
  //////////////////////////

  // unimplemented stub //

  ///////////////////////////
  // 2: Open SEM710 device //
  ///////////////////////////
  
  // use libUSB to detach the device from the kernel driver first, since 
  // FTDXX has no such functionality
  err = 0;
  libusb_context *context = NULL;
  libusb_init(&context);
  libusb_set_debug(context, 3);
  libusb_device_handle *handle;

  handle = libusb_open_device_with_vid_pid(context, vendor_id, product_id);
  
  if (libusb_kernel_driver_active(handle, 0)) {
    printf("Kernel driver already attached. Detaching...\n");
    err = libusb_detach_kernel_driver(handle, 0);
    if (err < 0) {
      printf("There was an error detaching the kernel driver.\n");
      return 1;
    }
    else {
      printf("Kernel driver detached.\n");
    }
  }
  libusb_release_interface(handle, 0);
  libusb_close(handle);
  libusb_exit(context);

  // now use FTDXX to open the device
  FT_STATUS ftStatus;
  FT_DEVICE_LIST_INFO_NODE *pDest = NULL;
  DWORD dwNumDevs;
  // unsigned char cBufWrite[BUF_SIZE];
  // char* pcBufLD[MAX_DEVICES + 1];
  // char cBufLD[MAX_DEVICES][64];
  
  // int i;
  FT_HANDLE ftHandle;

  /* for (i = 0; i < MAX_DEVICES; i++) { */
  /*   pcBufLD[i] = cBufLD[i]; */
  /* } */

  // pcBufLD[MAX_DEVICES] = NULL;

  ftStatus = FT_SetVIDPID((DWORD) vendor_id, (DWORD) product_id);
  if (ftStatus != FT_OK) {
    printf("Problem setting vID and pID\n");
    return 1;
  }

  ftStatus = FT_CreateDeviceInfoList(&dwNumDevs);
  if (ftStatus != FT_OK) {
    printf("Error: FT_CreateDeviceInfoList(%d)\n", (int) ftStatus);
    return 1;
  }

  ftStatus = FT_GetDeviceInfoList(pDest, &dwNumDevs);
  if (ftStatus != FT_OK) {
    printf("Error: FT_ListDevices(%d)\n", (int) ftStatus);
    return 1;
  }

  /* for (i = 0; (i < MAX_DEVICES) && (i < (int) dwNumDevs); i++) { */
  /*   printf("Device %d serial: %s\n", i, pDest[i].SerialNumber); */
  /* } */

  /* for (j = 0; j < BUF_SIZE; j++) { */
  /*   cBufWrite[j] = j; */
  /* } */

  ftStatus = FT_Open(0, &ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error: FT_Open(%d)\n", ftStatus);
    return 1;
  }

  ftStatus = FT_ResetDevice(ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error: FT_ResetDevice(%d)\n", ftStatus);
    return 1;
  }

  ftStatus = FT_SetBaudRate(ftHandle, SEM710_BAUDRATE);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetBaudRate(%d)\n", ftStatus);
    return 1;
  }

  ftStatus = FT_SetDataCharacteristics(ftHandle, 8, 0, 0);
  /*
    FT_DATA_BITS_8 == 8
    FT_STOP_BITS_1 == 0
    FT_PARITY_NONE == 0
  */
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetDataCharacteristics(%d)\n", ftStatus);
    return 1;
  }

  ftStatus = FT_SetFlowControl(ftHandle, 0x00, 0, 0);
  // FT_FLOW_NONE == &H0 == 0x00
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetFlowControl(%d)\n", ftStatus);
    return 1;
  }

  ftStatus = FT_SetTimeouts(ftHandle, 250, 250);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetTimeouts(%d)\n", ftStatus);
    return 1;
  }

  ftStatus = FT_SetLatencyTimer(ftHandle, 3);
  if (ftStatus != FT_OK) {
    printf("Error: FT_SetLatencyTimer(%d)\n", ftStatus);
    return 1;
  }

  // 2.5: report open status

  // 3: await instructions

  // 4: read data from SEM710

  // 4.5: transmit data from SEM710

  // 5: purge buffer; await further instructions

  return 0;
}
