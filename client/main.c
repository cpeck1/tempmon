#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include "./src/devstats.c"
#include "devtypes.h"
#include "./src/usb-operations.c"
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

  /* read_failed = SEM710_read_config_block(&ftHandle, &block, 1); */
  /* if (read_failed) { */
  /*   printf("Read config block failure.\n"); */
  /*   CONFIG_BLOCK_destroy(&block); */
  /*   FT_Close(ftHandle); */
  /*   return 1; */
  /* } */

  read_failed = SEM710_read_config(&ftHandle, &cal);
  if (read_failed) {
    printf("Read config failure.\n");
    CONFIG_BLOCK_destroy(&block);
    FT_Close(ftHandle);
    return 1;
  }

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
