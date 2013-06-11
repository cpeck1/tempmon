#define _GNU2_SOURCE

#include <stdio.h>
#include "./src/devstats.c"
#include "./src/usb-operations.c"

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
  return 0;
}


int main(void)
{
  /* 
     gimmicky, but validity of proceed checked at every step at which errors 
     might occur, where errors result in nullifying proceed; this way, on error
     the procedure skips the remaining procedures and hits the close and free
     statements at the end of main
  */
  int proceed;

  int product_id;
  int vendor_id;
  int err;

  struct ftdi_context *handle;

  int selection;
  SEM710_READINGS readings;
  CONFIG_DATA cal;
  uint8_t inc_buff[280];
  int inc_len;

  proceed = 1;
  err = 0;

  /* Get device stats from file: */
  err = get_device_ids(&product_id, &vendor_id);
  if (err) { proceed = 0; }

  /************************/
  /* 1: Connect to server */
  /************************/

  /* unimplemented stub */
 
  /*************************/
  /* 2: Open SEM710 device */
  /*************************/
  handle = ftdi_new();
  
  if (proceed) {
   err = detach_device_kernel(vendor_id, product_id);
    if (err) { proceed = 0; }
  }
  
  if (proceed) {
    err = open_device(handle, vendor_id, product_id);
    if (err) { proceed = 0; }
  }

  if (proceed) {
    err = prepare_device(handle);
    if (err) { proceed = 0; }
  }

  /***************************/
  /* 2.5: report open status */
  /***************************/
  
  /* Communication to server unimplemented */
 

  /****************************************/
  /* 3: await instructions (skip for now) */
  /****************************************/


  /* prompt user for selection */
  while (proceed) {
    printf("Device ready.\n");
    inc_len = -1;

    show_user_options();
    selection = get_user_selection();
    printf("\n");

    switch(selection) {
    case 0:
      proceed = 0;
      break;
    case 1: /* read process */
      inc_len = read_device(handle, SEM_COMMANDS_cREAD_PROCESS, inc_buff);
      if (inc_len < 0) {
	printf("Read process failure.\n");
	proceed = 0;
      }
      else {
	get_readings(&readings, inc_buff, inc_len);
	display_readings(&readings);
      }
      break;
    case 2: /* read config */
      inc_len = read_device(handle, SEM_COMMANDS_cREAD_CONFIG, inc_buff);
      if (inc_len < 0) {
    	printf("Read calibration failure.\n");
    	proceed = 0;
      }
      else {
	get_config_data(&cal, inc_buff, inc_len);
        display_config_data(&cal);
      }
      break;
    default:
      printf("Selection invalid.\n");
      break;
    }
    usleep(500000);
  }

  /**********************************/
  /* 4.5: transmit data from SEM710 */
  /**********************************/

  /* Communication to server unimplemented */

  /***********************************************/
  /* 5: purge buffer; await further instructions */
  /***********************************************/

  /*******************/
  /* 6: close device */
  /*******************/

  ftdi_usb_close(handle);
  ftdi_free(handle);
  
  return 0;
}
