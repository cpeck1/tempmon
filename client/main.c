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
  file_parse_test();
  array_test();

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
  struct ftdi_context *ftHandle;
  ftHandle = ftdi_new();
  
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
  int selection;
  int read_failed;
  /* int write_failed; */
  // FT_STATUS ftStatus;
  SEM710_READINGS readings;
  /* CONFIG_BLOCK block; */
  /* CONFIG_BLOCK_init(&block); */
  CONFIG_DATA cal;
  CONFIG_DATA_init(&cal);
  /* UNIVERSAL_CALIBRATION unical; */

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
      read_failed = SEM710_read_process(ftHandle, &readings);
      if (read_failed) {
	printf("Read process failure.\n");
	looping = 0;
      }
      else {
	SEM710_display_readings(&readings);
      }
      break;
    case 3: // read config
      read_failed = SEM710_read_config(ftHandle, &cal);
      if (read_failed) {
    	printf("Read calibration failure.\n");
    	looping = 0;
      }
      else {
        display_CONFIG_DATA(&cal);
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
