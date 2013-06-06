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

void show_user_options()
{
  printf("\nPlease enter number corresponding to selection.\n");
  printf("0: Exit\n");
  printf("1: Connect to host\n");
  printf("2: Get temperature reading\n");
  printf("3: edit calibration\n");
  printf("4: read calibration\n");
  printf("5: write calibration\n");
  printf("6: edit config\n");
  printf("7: read config\n");
  printf("8: write config\n");
}

int get_user_selection()
{
  char text[20];
  fputs("Enter selection: ", stdout);
  fflush(stdout);
  if ( fgets(text, sizeof text, stdin) )
    {
      int number;
      if ( sscanf(text, "%d", &number) == 1 )
	{
	  return number;
	}
    }
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
  int selection;
  int read_failed;
  int write_failed;
  FT_STATUS ftStatus;
  SEM710_READINGS readings;
  CONFIG_BLOCK block;
  CONFIG_BLOCK_init(&block);
  CONFIG_DATA cal;
  CONFIG_DATA_init(&cal);
  UNIVERSAL_CALIBRATION unical;

  // prompt user for selection
  int looping = 1;
  while (looping) {
    show_user_options();
    selection = get_user_selection();
    printf("\n");

    switch(selection) {
    case 0:
      looping = 0;
      break;
    case 1: // connect to server
      printf("Unimplemented\n");
      break;
    case 2: // read process
      read_failed = SEM710_read_process(&ftHandle, &readings);
      if (read_failed) {
	printf("Read process failure.\n");
	looping = 0;
      }
      else {
	SEM710_display_readings(&readings);
      }
      break;
    case 3: // edit calibration
      printf("Unimplemented\n");
      break;
    case 4: // read calibration
      read_failed = SEM710_read_cal(&ftHandle, &unical);
      if (read_failed) {
	printf("Read calibration failure.\n");
	looping = 0;
      }
      break;
    case 5: // write calibration
      write_failed = SEM710_set_cal(&ftHandle, &unical);
      if (write_failed) {
	printf("Write calibration failure.\n");
	looping = 0;
      }
      break;
    case 6: // edit configuration
      printf("Unimplemented\n");
      break;
    case 7: // read configuration
      read_failed = SEM710_read_config(&ftHandle, &cal);
      if (read_failed) {
	printf("Read config failure.\n");
	looping = 0;
      }
      break;
    case 8: // write configuration
      write_failed = SEM710_write_config(&ftHandle, &cal);
      if (write_failed) {
	printf("Write config failure.\n");
	looping = 0;
      }
      break;
    default:
      printf("Selection invalid.\n");
      break;
    }
    usleep(500000);
  }
  //////////////////////////////
  // 4: read data from SEM710 //
  //////////////////////////////


  // 4.5: transmit data from SEM710

  //////////////////////////////////////////
  // Communication to server unimplemented//
  //////////////////////////////////////////

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
