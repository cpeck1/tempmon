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

#define GLOBAL_FILE "globals.json"
#define READINGS_FILE "lastread.json"

int main(void)
{ 
  cJSON *froot;
  cJSON *fob;

  char *freezer_num = NULL;
  char *specifications_url = NULL;
  char *auth_user = NULL;
  char *auth_pwd = NULL;
  char *ca_path = NULL;

  cJSON *webroot;
  cJSON *webspecs;
  cJSON *ob;

  float read_frequency;
  char *upload_url_root = "";
  int product_id;
  int vendor_id;
  float expected_temperature;
  float safe_temperature_range;

  char upload_url[100];

  time_t tlast;
  time_t tnow;

  char *prev_status = "FIRST_WRITE_OK?";

  int err = 0;

  struct ftdi_context *ftHandle = NULL;

  int read_bytes;
  uint8_t inc_buf[280];

  SEM710_READINGS readings;

  time(&tlast);
  time(&tnow);

  ftHandle = ftdi_new();
  ftdi_init(ftHandle);

  curl_global_init(CURL_GLOBAL_ALL);

  /*************************/
  /* Get globals from file */
  /*************************/
  if ((froot = get_cjson_object_from_file(GLOBAL_FILE, 
					  "specifications")) == NULL) {
    err = 1;
  }
  else {
    fob = cJSON_GetObjectItem(froot, "freezer_num");
    if (fob != NULL) {
      freezer_num = fob->valuestring;
    } else { err = 1; }

    fob = cJSON_GetObjectItem(froot, "url");
    if (fob != NULL) {
      specifications_url = fob->valuestring;
    } else { err = 1; }

    fob = cJSON_GetObjectItem(froot, "user");
    if (fob != NULL) {
      auth_user = fob->valuestring;
    } else { err = 1; }

    fob = cJSON_GetObjectItem(froot, "pwd");
    if (fob != NULL) {
      auth_pwd = fob->valuestring;
    } else { err = 1; }

    fob = cJSON_GetObjectItem(froot, "ca_path");
    if (fob != NULL) {
      ca_path = fob->valuestring;
    } else { err = 1; }
  }

  if (err) {
    puts("Failed to fetch specifications from file. Exiting...");
    return 1;
  }

  while (1) {
    usleep(500000);
    err = 0;

    /*********************/
    /* Connect to server */
    /*********************/
    if ((webroot = get_runtime_specifications(specifications_url, auth_user,
					      auth_pwd, ca_path)) == NULL) {
      err = 1;
    }
    else {
      webspecs = cJSON_GetObjectItem(webroot, "specifications");
      if (webspecs != NULL) {
	ob = cJSON_GetObjectItem(webspecs, "read_frequency");
	if (ob != NULL) {
	  read_frequency = ob->valuedouble;
	} else { err = 1; }

	ob = cJSON_GetObjectItem(webspecs, "upload_url_root");
	if (ob != NULL) {
	  upload_url_root = ob->valuestring;
	} else { err = 1; }

	ob = cJSON_GetObjectItem(webspecs, "product_id");
	if (ob != NULL) {
	  product_id = ob->valueint;
	} else { err = 1; }

	ob = cJSON_GetObjectItem(webspecs, "vendor_id");
	if (ob != NULL) {
	  vendor_id = ob->valueint;
	} else { err = 1; }

	ob = cJSON_GetObjectItem(webspecs, "expected_temperature");
	if (ob != NULL) {
	  expected_temperature = ob->valuedouble;
	} else { err = 1; }

	ob = cJSON_GetObjectItem(webspecs, "safe_temperature_range");
	if (ob != NULL) {
	  safe_temperature_range = ob->valuedouble;
	} else { err = 1; }
      }

      if (!err) {
	strcat(upload_url_root, freezer_num);
	strcpy(upload_url, upload_url_root);

	read_frequency = read_frequency - 0.066666; 
      }
      cJSON_Delete(webroot);
    }

    if (err) {
      puts("Failed to fetch runtime specifications from server. Retrying...");
      continue;
    }

    /**********************/
    /* Open SEM710 device */
    /**********************/
    err = detach_device_kernel(vendor_id, product_id);
    if (err) {
      printf("Error detaching device kernel. Is the device attached?\n");
      continue; 
    }

    err = ftdi_usb_open(ftHandle, vendor_id, product_id);
    if (err) { 
      printf("Error opening device.\n");
      continue; 
    }

    err = prepare_device(ftHandle);
    if (err) { 
      printf("Error preparing device.\n");
      continue;
    }

    /*************************/
    /* read data from SEM710 */
    /*************************/
    read_bytes = read_device(ftHandle, SEM_COMMANDS_cREAD_PROCESS, inc_buf);
    ftdi_usb_close(ftHandle);
    if (read_bytes <= 0) {
      printf("Read process failure.\n");
      continue;
    }
    get_readings(&readings, expected_temperature, safe_temperature_range,
		 inc_buf, read_bytes);

    /*************************/
    /* Upload data to server */
    /*************************/
    time(&tnow);
    if (((difftime(tnow, tlast)/60) > read_frequency ||
	 (prev_status != readings.STATUS)) && read_bytes > 0) {
      /*
	if the time since the last read is greater than the required wait time,
	or if the status has changed since the last read, update.
      */
      pack_readings(&readings, READINGS_FILE);
      do_web_put(upload_url, READINGS_FILE, auth_user, auth_pwd, ca_path);
      time(&tlast);
    }
    prev_status = readings.STATUS;
  }

  /* close device */
  cJSON_Delete(froot);
  ftdi_deinit(ftHandle);
  ftdi_free(ftHandle);
  return 0;
}

#endif
