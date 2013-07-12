#ifndef __MAIN_FN
#define __MAIN_FN

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "fparse.h"
#include "devtypes.h"
#include "usb-operations.h"
#include "http-operations.h"
#include "cJSON.h"

#define BUF_SIZE 0x10
#define MAX_DEVICES 1
#define SPEC_FILE "product.json"
#define READINGS_FILE "lastread.json"

int main(void)
{ 
  char *freezer_num = NULL;
  char *specifications_url = NULL;
  char *auth_user = NULL;
  char *auth_pwd = NULL;

  float read_frequency;
  char *upload_url_root;
  int product_id;
  int vendor_id;

  time_t tlast;
  time_t tnow;

  char *prev_status = "FIRST_WRITE_OK?";

  int err;
  int looping;

  struct ftdi_context *ftHandle;

  int read_bytes;
  uint8_t inc_buf[280];

  SEM710_READINGS readings;
  /* CONFIG_DATA cal; */

  time(&tlast);
  time(&tnow);

  ftHandle = ftdi_new();

  curl_global_init(CURL_GLOBAL_ALL);
  if (get_specifications("product.json", &freezer_num, &specifications_url,
  			 &auth_user, &auth_pwd)) {
    puts("Failed to fetch specifications from file. Exiting...");
    return 1;
  }

  looping = 1;
  while (looping) {
    usleep(500000);
    time(&tnow);
    /************************/
    /* 1: Connect to server */
    /************************/
    if (get_runtime_specifications(specifications_url, auth_user, auth_pwd,
				   &read_frequency, &upload_url_root,
				   &product_id, &vendor_id)) {
      puts("Failed to fetch runtime specifications from server. Retrying...");
      continue;
    }
    strcat(upload_url_root, freezer_num);

    /*************************/
    /* 2: Open SEM710 device */
    /*************************/

    err = detach_device_kernel(vendor_id, product_id);
    if (err) {
      printf("Error detaching device kernel. Is the device attached?\n");
      continue; 
    }

    err = open_device(ftHandle, vendor_id, product_id);
    if (err) { 
      printf("Error opening device.\n");
      continue; 
    }

    err = prepare_device(ftHandle);
    if (err) { 
      printf("Error preparing device.\n");
      continue;
    }

    /* printf("Device prepared.\n"); */
    read_bytes = read_device(ftHandle, SEM_COMMANDS_cREAD_PROCESS, inc_buf);
    get_readings(&readings, inc_buf, read_bytes);
    ftdi_usb_close(ftHandle);
    if (read_bytes <= 0) {
      printf("Read process failure.\n");
      continue;
    }
    else if ((difftime(tnow, tlast)/60) > read_frequency ||
	     (prev_status != readings.STATUS)){
      /* display_readings(&readings); */
      pack_readings(&readings, READINGS_FILE);
      do_web_put(upload_url_root, READINGS_FILE, auth_user, auth_pwd);
      time(&tlast);
    }
    prev_status = readings.STATUS;
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
