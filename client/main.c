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
  printf("asca");
  libusb_context* ctx;
  printf("asda");
  libusb_device_handle* device_handle = libusb_open_device_with_vid_pid(ctx,
  						     vendor_id, product_id);
  printf("asdb");
  libusb_detach_kernel_driver(device_handle, 0);
  printf("asdc");
  libusb_close(device_handle);
  printf("asdd");

  // now use FTDXX to open the device
  FT_STATUS ftStatus;
  DWORD dwNumDevs;
  unsigned char cBufWrite[BUF_SIZE];
  char* pcBufLD[MAX_DEVICES + 1];
  char cBufLD[MAX_DEVICES][64];
  int iDevicesOpen = 0;
  
  int i, j;
  FT_HANDLE ftHandle;
  DWORD pID = product_id;
  DWORD vID = vendor_id;

  for (i = 0; i < MAX_DEVICES; i++) {
    pcBufLD[i] = cBufLD[i];
  }
  pcBufLD[MAX_DEVICES] = NULL;

  ftStatus = FT_SetVIDPID(vID, pID);
  if (ftStatus != FT_OK) {
    printf("Problem setting vID and pID\n");
    return 1;
  }

  ftStatus = FT_CreateDeviceInfoList(&dwNumDevs);
  if (ftStatus != FT_OK) {
    printf("Error: FT_CreateDeviceInfoList(%d)\n", (int) ftStatus);
    return 1;
  }

  printf("numdevs: %d\n", dwNumDevs);

  ftStatus = FT_ListDevices(pcBufLD, &dwNumDevs, FT_LIST_ALL);
  if (ftStatus != FT_OK) {
    printf("Error: FT_ListDevices(%d)\n", (int) ftStatus);
    return 1;
  }

  for (i = 0; (i < MAX_DEVICES) && (i < (int) dwNumDevs); i++) {
    printf("Device %d Serial Number - %s\n", i, cBufLD[i]);
  }

  for (j = 0; j < BUF_SIZE; j++) {
    cBufWrite[j] = j;
  }

  ftStatus = FT_Open(0, &ftHandle);
  if (ftStatus != FT_OK) {
    printf("Error: FT_Open(%d)\n", ftStatus);
    return 1;
  }

  printf("Opened device %s\n", cBufLD[i]);
  iDevicesOpen++;

  // 2.5: report open status

  // 3: await instructions

  // 4: read data from SEM710

  // 4.5: transmit data from SEM710

  // 5: purge buffer; await further instructions

  return 0;
}
