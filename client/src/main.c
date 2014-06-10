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

#define GLOBAL_FILE "globals.ini"
#define READINGS_FILE "lastread.json"
#define AUTH_FILE "auth.json"

#define IO_ERROR 1
#define USB_OPEN_ERROR 2
#define USB_READ_ERROR 3
#define SERVER_ERROR 4

int main(void)
{
  char fbuffer[256];
  char *vfnd;

  char *container_num = (char *) malloc(64);
  char *specifications_url = (char *) malloc(256);
  char *specifications_uri = (char *) malloc(128);
  char *auth_user = (char *) malloc(256);
  char *auth_pwd = (char *) malloc(256);
  char *ca_path = (char *) malloc(256);

  char *specifications_path = (char *) malloc(256);

  cJSON *webroot;
  cJSON *webspecs;
  cJSON *ob;
  cJSON *monitor;

  int next_update_in;
  char *upload_url_root = (char *) malloc(128);
  int product_id;
  int vendor_id;
  float expected_temperature;
  float safe_temperature_range;

  char *upload_url = (char *) malloc(524);

  time_t tlast;
  time_t tnow;

  char *prev_status = (char *) malloc(256);

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

  vfnd = get_file_variable(GLOBAL_FILE, "url", fbuffer);
  if (vfnd) {
    strcpy(specifications_url, fbuffer);
  } else {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
  

    return IO_ERROR;
  }

  vfnd = get_file_variable(GLOBAL_FILE, "container_num", fbuffer);
  if (vfnd) {
    strcpy(container_num, fbuffer);
  } else {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
  

    return IO_ERROR;
  }
  
  vfnd = get_file_variable(GLOBAL_FILE, "spec_uri", fbuffer);
  if (vfnd) {
    strcpy(specifications_uri, fbuffer);
  } else {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return IO_ERROR;
  }
    
  vfnd = get_file_variable(GLOBAL_FILE, "user", fbuffer);
  if (vfnd) {
    strcpy(auth_user, fbuffer);
  } else {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return IO_ERROR;
  }
  
  vfnd = get_file_variable(GLOBAL_FILE, "password", fbuffer);
  if (vfnd) {
    strcpy(auth_pwd, fbuffer);
  } else {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return IO_ERROR;
  }
  

  /**********************************/
  /* Get specifications from server */
  /**********************************/
  pack_auth(auth_user, auth_pwd, AUTH_FILE);
  sprintf(specifications_path, "%s%s%s", specifications_url, container_num, 
	  specifications_uri);

  ca_path = NULL;
  if ((webroot = get_runtime_specifications(specifications_path, 
					    AUTH_FILE, 
					    ca_path)) == NULL) {
    err = 1;
  }
  else {
    webspecs = cJSON_GetObjectItem(webroot, "specifications");
    
    if (webspecs != NULL) {
      ob = cJSON_GetObjectItem(webspecs, "nextUpdateIn");
      if (ob != NULL) {
	next_update_in = atoi(ob->valuestring);
      } else { 
	err = 1; 
      }
      
      ob = cJSON_GetObjectItem(webspecs, "uploadURL");
      if (ob != NULL) {	
	upload_url_root = ob->valuestring;
	
      } else { 
	err = 1; 
      }
      
      monitor = cJSON_GetObjectItem(webspecs, "monitor");
      if (monitor != NULL) {
	ob = cJSON_GetObjectItem(monitor, "productID");
	if (ob != NULL) {
	  product_id = atoi(ob->valuestring);
	} else { 
	  err = 1; 
	}
	
	ob = cJSON_GetObjectItem(monitor, "vendorID");
	if (ob != NULL) {
	  vendor_id = atoi(ob->valuestring);
	} else { 
	  err = 1; 
	}
      }
      else {
	err = 1;
      }
      
      ob = cJSON_GetObjectItem(webspecs, "expectedTemperature");
      if (ob != NULL) {
	expected_temperature = atof(ob->valuestring);
      } else { 
	err = 1; 
      }
      
      ob = cJSON_GetObjectItem(webspecs, "temperatureRange");
      if (ob != NULL) {
	safe_temperature_range = atof(ob->valuestring);
      } else { 
	err = 1; 
      }
    }
    if (!err) {
      strcpy(upload_url, specifications_url);
      strcat(upload_url, upload_url_root);      
    }
    cJSON_Delete(webroot);
  }
  if (err) {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return SERVER_ERROR;
  }

  err = 0;

  /************************/
  /* Detach device kernel */
  /************************/
  err = detach_device_kernel(vendor_id, product_id);
  if (err) {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return USB_OPEN_ERROR;
  }

  /***************/
  /* Open device */
  /***************/
  err = ftdi_usb_open(ftHandle, vendor_id, product_id);
  if (err) { 
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return USB_OPEN_ERROR;
  }

  /******************/
  /* Prepare device */
  /******************/
  err = prepare_device(ftHandle);
  if (err) { 
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return USB_OPEN_ERROR;
  }
  
  /*******************************/
  /* Get previous reading status */
  /*******************************/
  err = get_cjson_object_from_file("lastread.json", "status", &prev_status);
  
  if (err) {
    prev_status = "FIRST_WRITE_OK";
  } 

  /*************************/
  /* read data from SEM710 */
  /*************************/
  read_bytes = read_device(ftHandle, SEM_COMMANDS_cREAD_PROCESS, inc_buf);
  ftdi_usb_close(ftHandle);
  
  if (read_bytes <= 0) {
    ftdi_deinit(ftHandle);
    ftdi_free(ftHandle);
  
    free(container_num);
    free(specifications_url);
    free(specifications_uri);
    free(specifications_path);
    free(auth_user);
    free(auth_pwd);
    free(ca_path);
    free(upload_url);
    return USB_READ_ERROR;
  }
  
  get_readings(&readings, expected_temperature, safe_temperature_range,
	       inc_buf, read_bytes);
  
  /*************************/
  /* Upload data to server */
  /*************************/
  if ((next_update_in <= 0 || strcmp(prev_status, readings.STATUS)) && 
      read_bytes > 0) {
    /*
      if the time since the last read is greater than the required wait time,
      or if the status has changed since the last read, update.
    */
    err = pack_readings(&readings, auth_user, auth_pwd, READINGS_FILE);

    if (err) {
      ftdi_deinit(ftHandle);
      ftdi_free(ftHandle);
  
      free(container_num);
      free(specifications_url);
      free(specifications_uri);
      free(specifications_path);
      free(auth_user);
      free(auth_pwd);
      free(ca_path);
      free(upload_url);
      return IO_ERROR;
    }
    do_http_put(upload_url, READINGS_FILE, ca_path);
  }
  
  /****************/
  /* close device */
  /****************/
  ftdi_deinit(ftHandle);
  ftdi_free(ftHandle);
  
  free(container_num);
  free(specifications_url);
  free(specifications_uri);
  free(specifications_path);
  free(auth_user);
  free(auth_pwd);
  free(ca_path);
  free(upload_url);
  
  return 0;
}

#endif
