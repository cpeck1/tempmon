#ifndef MAIN_FN
#define MAIN_FN

#include <stdio.h>

#include "fparse.h"
#include "devtypes.h"
#include "usb-operations.h"
#include "http-operations.h"
#include "cJSON.h"

#define BUF_SIZE 0x10
#define MAX_DEVICES 1
#define SPEC_FILE "product.json"
#define READINGS_FILE "lastread.json"

void show_user_options()
{
  printf("\nPlease enter number corresponding to selection.\n");
  printf("0: Exit\n");
  printf("1: Get temperature reading\n");
  printf("2: Read device config\n");
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
  char *freezer_num;
  char *specifications_url;
  char *auth_user;
  char *auth_pwd;

  float read_frequency;
  char *upload_url_root;
  int product_id;
  int vendor_id;

  int err;
  int looping;

  struct ftdi_context *ftHandle;

  int read_bytes;
  uint8_t inc_buf[280];

  SEM710_READINGS readings;
  /* CONFIG_DATA cal; */


  ftHandle = ftdi_new();

  curl_global_init(CURL_GLOBAL_ALL);

  if (get_specifications(SPEC_FILE, &freezer_num, &specifications_url,
  			 &auth_user, &auth_pwd)) {
    return 1;
  }

  looping = 1;
  while (looping) {
    if (get_specifications(SPEC_FILE, &freezer_num, &specifications_url,
			   &auth_user, &auth_pwd)) {
      return 1;
    }
    /************************/
    /* 1: Connect to server */
    /************************/
    if (get_runtime_specifications(specifications_url, auth_user, auth_pwd,
				   &read_frequency, &upload_url_root,
				   &product_id, &vendor_id)) {
      return 1;
    }

    strcat(upload_url_root, freezer_num);

    /*************************/
    /* 2: Open SEM710 device */
    /*************************/

    err = detach_device_kernel(vendor_id, product_id);
    if (err) {
      printf("error detaching device kernel\n");
      return 1; 
    }

    err = open_device(ftHandle, vendor_id, product_id);
    if (err) { 
      printf("error opening device \n");
      return 1; 
    }

    err = prepare_device(ftHandle);
    if (err) { 
      printf("error preparing device \n");
      return 1;
    }

    printf("Device prepared.\n");
    read_bytes = read_device(ftHandle, SEM_COMMANDS_cREAD_PROCESS, inc_buf);
    ftdi_usb_close(ftHandle);
    if (read_bytes <= 0) {
      printf("Read process failure.\n");
      looping = 0;
    }
    else {
      get_readings(&readings, inc_buf, read_bytes);
      display_readings(&readings);
      pack_readings(&readings, READINGS_FILE);
      do_web_put(upload_url_root, READINGS_FILE, auth_user, auth_pwd);
    }
    /* case 2:  /\* read config *\/ */
    /*   read_bytes = read_device(ftHandle, SEM_COMMANDS_cREAD_CONFIG, inc_buf); */
    /*   if (read_bytes <= 0) { */
    /* 	printf("Read calibration failure.\n"); */
    /* 	looping = 0; */
    /*   } */
    /*   else { */
    /*     get_config(&cal, inc_buf, read_bytes); */
    /* 	display_config(&cal); */
    /*   } */
    /*   break; */
    /* default: */
    /*   printf("Selection invalid.\n"); */
    /*   break; */
    /* } */
    usleep(500000);
  }
  /****************************/
  /* 4: read data from SEM710 */
  /****************************/

  /* 4.5: transmit data from SEM710 */

  /****************************************/
  /* Communication to server unimplemented*/
  /****************************************/

  /* 5: purge buffer; await further instructions */

  /* 6: close device */

  ftdi_free(ftHandle);
  return 0;
}

#endif
