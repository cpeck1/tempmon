#ifndef __MAIN_FN
#define __MAIN_FN

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "fparse.h"
#include "devtypes.h"
#include "usb-operations.h"
#include "http-operations.h"
#include "cJSON.h"

#define GLOBAL_FILE "globals.json"
#define READINGS_FILE "lastread.json"
#define AUTH_FILE "auth.json"

int main(void)
{ 
  cJSON *froot;
  cJSON *fob;

  char *freezer_num = (char *) malloc(50);
  char *specifications_url = (char *) malloc(250);
  char *specifications_uri = (char *) malloc(100);
  char *auth_user = (char *) malloc(255);
  char *auth_pwd = (char *) malloc(255);
  char *ca_path = (char *) malloc(255);

  char *specifications_path = (char *) malloc(255);

  cJSON *webroot;
  cJSON *webspecs;
  cJSON *ob;
  cJSON *monitor;

  float next_update_in;
  char *upload_url_root = (char *) malloc(100);
  int product_id;
  int vendor_id;
  float expected_temperature;
  float safe_temperature_range;

  char *upload_url = (char *) malloc(500);

  time_t tlast;
  time_t tnow;

  char *prev_status = "FIRST_WRITE_OK?";

  int err = 0;
  int specs_not_updated = 1;

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

    fob = cJSON_GetObjectItem(froot, "spec_uri");
    if (fob != NULL) {
      specifications_uri = fob->valuestring;
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


  pack_auth(auth_user, auth_pwd, AUTH_FILE);
  sprintf(specifications_path, "%s%s%s", specifications_url, freezer_num, specifications_uri);
  //strcpy(specifications_path, specifications_url);
  //strcat(specifications_path, freezer_num);
  //strcat(specifications_path, specifications_uri);
  ca_path = NULL;
  
  while (specs_not_updated) {
    if (
	(webroot = get_runtime_specifications(specifications_path, 
					      AUTH_FILE, 
					      ca_path)) == NULL) {
      err = 1;
    }
    else {
      webspecs = cJSON_GetObjectItem(webroot, "specifications");
    
      if (webspecs != NULL) {
	ob = cJSON_GetObjectItem(webspecs, "nextUpdateIn");
	if (ob != NULL) {
	  next_update_in = atof(ob->valuestring);
	} else { 
	  puts("Missing specifications parameter: nextUpdateIn\n");
	  err = 1; 
	}
      
	ob = cJSON_GetObjectItem(webspecs, "uploadURL");
	if (ob != NULL) {	
	  upload_url_root = ob->valuestring;
	
	} else { 
	  puts("Missing specifications parameter: uploadURL");
	  err = 1; 
	}
      
	monitor = cJSON_GetObjectItem(webspecs, "monitor");
	if (monitor != NULL) {
	  ob = cJSON_GetObjectItem(monitor, "productID");
	  if (ob != NULL) {
	    product_id = atoi(ob->valuestring);
	  } else { 
	    puts("Missing specifications parameter: productID");
	    err = 1; 
	  }
	
	  ob = cJSON_GetObjectItem(monitor, "vendorID");
	  if (ob != NULL) {
	    vendor_id = atoi(ob->valuestring);
	  } else { 
	    err = 1; 
	    puts("Missing specifications parameter: vendorID");
	  }
	}
	else {
	  puts("Missing specifications parameter: monitor");
	  err = 1;
	}
      
	ob = cJSON_GetObjectItem(webspecs, "expectedTemperature");
	if (ob != NULL) {
	  expected_temperature = atof(ob->valuestring);
	} else { 
	  puts("Missing specifications parameter: expectedTemperature");
	  err = 1; 
	}
      
	ob = cJSON_GetObjectItem(webspecs, "temperatureRange");
	if (ob != NULL) {
	  safe_temperature_range = atof(ob->valuestring);
	} else { 
	  puts("Missing specifications parameter: temperatureRange");
	  err = 1; 
	}
      }
      if (!err) {
	strcpy(upload_url, specifications_url);
	strcat(upload_url, upload_url_root);
      
      }
      cJSON_Delete(webroot);
      specs_not_updated = 0;
    }
    if (err) {
      puts("Failed to fetch runtime specifications from server. Retrying...");
      usleep(5000000);
    }
  }  
  while (1) {
    usleep(5000000);
    next_update_in = next_update_in - 5;
    err = 0;
    /**********************/
    /* Open SEM710 device */
    /**********************/
    err = detach_device_kernel(vendor_id, product_id);
    if (err) {
      printf("Error detaching kernel of device with vendor ID %d and product ID %d. Is the device attached?\n", vendor_id, product_id);
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
    if ((next_update_in == 0 ||
	 (prev_status != readings.STATUS)) && read_bytes > 0) {
      /*
	if the time since the last read is greater than the required wait time,
	or if the status has changed since the last read, update.
      */
      pack_readings(&readings, auth_user, auth_pwd, READINGS_FILE);
      do_web_put(upload_url, READINGS_FILE, ca_path);

      specs_not_updated = 1;
      while (specs_not_updated) {
	if (
	    (webroot = get_runtime_specifications(specifications_path, 
						  AUTH_FILE, 
						  ca_path)) == NULL) {
	  err = 1;
	}
	else {
	  webspecs = cJSON_GetObjectItem(webroot, "specifications");
	
	  if (webspecs != NULL) {
	    ob = cJSON_GetObjectItem(webspecs, "nextUpdateIn");
	    if (ob != NULL) {
	      next_update_in = atof(ob->valuestring);
	      printf("next update in = %f\n", next_update_in);
	    } else { 
	      puts("Missing specifications parameter: nextUpdateIn\n");
	      err = 1; 
	    }
      
	    ob = cJSON_GetObjectItem(webspecs, "uploadURL");
	    if (ob != NULL) {	
	      upload_url_root = ob->valuestring;
	
	      printf("upload url root = %s\n", upload_url_root);
	    } else { 
	      puts("Missing specifications parameter: uploadURL");
	      err = 1; 
	    }
      
	    monitor = cJSON_GetObjectItem(webspecs, "monitor");
	    if (monitor != NULL) {
	      ob = cJSON_GetObjectItem(monitor, "productID");
	      if (ob != NULL) {
		product_id = atoi(ob->valuestring);
		printf("product ID: %d\n", product_id);
	      } else { 
		puts("Missing specifications parameter: productID");
		err = 1; 
	      }
	
	      ob = cJSON_GetObjectItem(monitor, "vendorID");
	      if (ob != NULL) {
		vendor_id = atoi(ob->valuestring);
		printf("vendor ID: %d\n", vendor_id);
	      } else { 
		err = 1; 
		puts("Missing specifications parameter: vendorID");
	      }
	    }
	    else {
	      puts("Missing specifications parameter: monitor");
	      err = 1;
	    }
      
	    ob = cJSON_GetObjectItem(webspecs, "expectedTemperature");
	    if (ob != NULL) {
	      expected_temperature = atof(ob->valuestring);
	      printf("expected temperature = %f\n", expected_temperature);
	    } else { 
	      puts("Missing specifications parameter: expectedTemperature");
	      err = 1; 
	    }
      
	    ob = cJSON_GetObjectItem(webspecs, "temperatureRange");
	    if (ob != NULL) {
	      safe_temperature_range = atof(ob->valuestring);
	      printf("temperature range = %f\n", safe_temperature_range);
	    } else { 
	      puts("Missing specifications parameter: temperatureRange");
	      err = 1; 
	    }
	  }
	  if (!err) {
	    strcpy(upload_url, specifications_url);
	    strcat(upload_url, upload_url_root);
      
	  }
	  cJSON_Delete(webroot);
	  specs_not_updated = 0;
	}
	if (err) {
	  puts("Failed to fetch runtime specifications from server. Retrying...");
	}
      }
    }
    prev_status = readings.STATUS;
  }

  /* close device */
  cJSON_Delete(froot);
  ftdi_deinit(ftHandle);
  ftdi_free(ftHandle);
  
  free(freezer_num);
  free(specifications_url);
  free(specifications_uri);
  free(specifications_path);
  free(auth_user);
  free(auth_pwd);
  free(ca_path);
  free(upload_url);
}

#endif
