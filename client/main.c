#define _GNU2_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <ftdi.h>

#include "src/fparse.c"
#include "src/devtypes.h"
#include "src/usb-operations.c"

#include "tests/array_t.c"
#include "tests/fparse_t.c"

#define BUF_SIZE 0x10
#define MAX_DEVICES 1

void show_user_options()
{
  printf("\nPlease enter number corresponding to selection.\n");
  printf("0: Exit\n");
  printf("1: Connect to host\n");
  printf("2: Get temperature reading\n");
  printf("3: Read device config\n");
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
  int product_id;
  int vendor_id;

  int err = 0;

  int detach_failed;
  int open_failed;
  int prep_failed;
  struct ftdi_context *ftHandle;

  int selection;
  int read_bytes;

  SEM710_READINGS readings;
  CONFIG_DATA cal;

  uint8_t inc_buf[280];

  int looping = 1;

  file_parse_test();
  array_test();

  // Get device ids from file:
  ftHandle = ftdi_new();

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

  
  detach_failed = detach_device_kernel(vendor_id, product_id);
  if (detach_failed) { return 1; }
  
  open_failed = open_device(ftHandle, vendor_id, product_id);
  if (open_failed) { return 1; }

  prep_failed = prepare_device(ftHandle);
  if (prep_failed) { return 1; }

  // 2.5: report open status
  
  ///////////////////////////////////////////
  // Communication to server unimplemented //
  ///////////////////////////////////////////
  printf("Device prepared.\n");

  //////////////////////////////////////////
  // 3: await instructions (skip for now) //
  //////////////////////////////////////////

  while (looping) {
    show_user_options();
    selection = get_user_selection();
    printf("\n");

    switch(selection) {
    case 0:
      looping = 0;
      break;
    case 1: // read process
      read_bytes = read_device(ftHandle, SEM_COMMANDS_cREAD_PROCESS, inc_buf);
      if (read_bytes < 0) {
	printf("Read process failure.\n");
	looping = 0;
      }
      else {
        get_readings(&readings, inc_buf, read_bytes);
	display_readings(&readings);
      }
      break;
    case 2: // read config
      read_bytes = read_device(ftHandle, SEM_COMMANDS_cREAD_CONFIG, inc_buf);
      if (read_bytes < 0) {
    	printf("Read calibration failure.\n");
    	looping = 0;
      }
      else {
        get_config(&cal, inc_buf, read_bytes);
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
  ftdi_usb_close(ftHandle);
  ftdi_free(ftHandle);
  
  return 0;
}
