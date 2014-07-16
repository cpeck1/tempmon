#ifndef __MAIN_FN
#define __MAIN_FN

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <openssl/ssl.h>

#include "fparse.h"
#include "devtypes.h"
#include "usb-operations.h"
#include "http-operations.h"
#include "misc-structs.h"
#include "cJSON.h"

#define GLOBAL_FILE "globals.ini"
#define READINGS_FILE "/tmp/lastread.json"
#define AUTH_FILE "/tmp/auth.json"
#define URL_FILE "/tmp/SERVER_URL"
#define COOKIE_FILE "/tmp/SERVER_COOKIE"

#define IO_ERROR 1
#define USB_OPEN_ERROR 2
#define USB_READ_ERROR 3
#define SERVER_ERROR 4

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

void deinit_string(struct string *s) {
  free(s->ptr);
}

int main(void)
{
  char fbuffer[256];

  char *container_num = (char *) malloc(63);
  char *last_read_status = (char *) malloc(255);

  char *server_login_email = (char *) malloc(255);
  char *server_login_password = (char *) malloc(255);
  
  char *server_path_authentication = (char *) malloc(255);
  char *server_path_containers = (char *) malloc(255);
  char *server_path_specifications = (char *) malloc(255);
  char *server_path_reading_upload = (char *) malloc(255);

  char *certificate_authority_path = (char *) malloc(255);

  char *server_url_base = (char *) malloc(255);
  char *server_url_authentication = (char *) malloc(255);
  char *server_url_specifications = (char *) malloc(255);
  char *server_url_reading_upload = (char *) malloc(255);

  char *postfield_buffer = (char *) malloc(1023);
  char *error_buffer = (char *) malloc(255);

  int http_response_code;
  struct string http_response_buffer;
  init_string(&http_response_buffer);
  
  struct string http_response_jsobj;
  init_string(&http_response_jsobj);

  cJSON *JSON_root;
  cJSON *JSON_specifications;
  cJSON *ob;
  cJSON *monitor;

  int time_until_update;
  int device_product_id;
  int device_vendor_id;

  int err = 0;

  struct ftdi_context *ftHandle = NULL;

  int read_bytes;
  uint8_t reading_buffer[280];

  SEM710_READINGS readings;

  ftHandle = ftdi_new();
  ftdi_init(ftHandle);

  curl_global_init(CURL_GLOBAL_ALL);

  /*************************/
  /* Get globals from file */
  /*************************/
  printf("Getting globals from file... ");
  err = get_file_variable(URL_FILE, "url", fbuffer);
  if (!err) {
    strcpy(server_url_base, fbuffer);
  } else {
    puts("variable not found \"url\"");
    exit(IO_ERROR);
  }

  err = get_file_variable(GLOBAL_FILE, "container_num", fbuffer);
  if (!err) {
    strcpy(container_num, fbuffer);
  } else {
    puts("variable not found \"container_num\"");
    exit(IO_ERROR);
  }

  err = get_file_variable(GLOBAL_FILE, "authentication_path", fbuffer);
  if (!err) {
    strcpy(server_path_authentication, fbuffer);
  } else {
    puts("variable not found \"authentication_path\"");
    exit(IO_ERROR);
  }

  err = get_file_variable(GLOBAL_FILE, "container_path", fbuffer);
  if (!err) {
    strcpy(server_path_containers, fbuffer);
  } else {
    puts("variable not found \"container_path\"");
    exit(IO_ERROR);
  }
  
  err = get_file_variable(GLOBAL_FILE, "specifications_path", fbuffer);
  if (!err) {
    strcpy(server_path_specifications, fbuffer);
  } else {
    puts("variable not found \"specifications_path\"");
    exit(IO_ERROR);
  }
    
  err = get_file_variable(GLOBAL_FILE, "user", fbuffer);
  if (!err) {
    strcpy(server_login_email, fbuffer);
  } else {
    puts("variable not found \"user\"");
    exit(IO_ERROR);
  }
  
  err = get_file_variable(GLOBAL_FILE, "password", fbuffer);
  if (!err) {
    strcpy(server_login_password, fbuffer);
  } else {
    puts("variable not found \"password\"");
    exit(IO_ERROR);
  }
  
  printf("done\n");
  /******************************/
  /* Authenticating with server */
  /******************************/
  printf("Authenticating with server... ");
  sprintf(server_url_authentication,
	  "%s%s",
	  server_url_base,
	  server_path_authentication);

  /* construct login postfield */
  start_postfield(postfield_buffer, "email", server_login_email);
  add_postfield(postfield_buffer, "password", server_login_password);
  
  /* perform a POST operation to the server authentication page */
  http_response_code = http_POST(server_url_authentication,
				 NULL,
				 COOKIE_FILE,
				 NULL,
				 postfield_buffer,
				 &http_response_buffer);

  if (http_response_code != 0) {
    printf("done\n");
  }
  else {
    printf("no response from server.\n");
    exit(SERVER_ERROR);
  }
 
  /**********************************/
  /* Get specifications from server */
  /**********************************/
  printf("Getting specifications from server... ");

  sprintf(server_url_specifications, 
	  "%s%s/%s%s", 
	  server_url_base, 
	  server_path_containers,
	  container_num,
	  server_path_specifications);

  http_response_code = http_GET(server_url_specifications,
				COOKIE_FILE,
				NULL,
				&http_response_jsobj);
  
  JSON_root = cJSON_Parse(http_response_jsobj.ptr);
  if (JSON_root == NULL) {
    printf("invalid JSON struct returned\n");
    err = 1;
  }
  else {
    JSON_specifications = cJSON_GetObjectItem(JSON_root, "specifications");

    if (JSON_specifications != NULL) {
      ob = cJSON_GetObjectItem(JSON_specifications, "nextUpdateIn");
      if (ob != NULL) {
	time_until_update = atoi(ob->valuestring);
      } else { 
	puts("variable not found \"nextUpdateIn\"");
	err = 1; 
      }
      
      ob = cJSON_GetObjectItem(JSON_specifications, "lastReadStatus");
      if (ob != NULL) {
	last_read_status = ob->valuestring;
      } else {
	puts("variable not found \"lastReadStatus\""); 
	err = 1;
      }

      ob = cJSON_GetObjectItem(JSON_specifications, "uploadURL");
      if (ob != NULL) {	
	server_path_reading_upload = ob->valuestring;
      } else { 
	puts("variable not found \"uploadURL\"");
	err = 1; 
      }
      
      monitor = cJSON_GetObjectItem(JSON_specifications, "monitor");
      if (monitor != NULL) {
	ob = cJSON_GetObjectItem(monitor, "productID");
	if (ob != NULL) {
	  device_product_id = atoi(ob->valuestring);
	} else { 
	  puts("variable not found \"monitor\"");
	  err = 1; 
	}
	
	ob = cJSON_GetObjectItem(monitor, "vendorID");
	if (ob != NULL) {
	  device_vendor_id = atoi(ob->valuestring);
	} else { 
	  puts("variable not found \"vendorID\"");
	  err = 1; 
	}
      }
      else {
	puts("JSON struct not given by server");
	err = 1;
      }
    }
    if (!err) {
      strcpy(server_url_reading_upload, server_url_base);
      strcat(server_url_reading_upload, server_path_reading_upload);      
    }
    cJSON_Delete(JSON_root);
  }
  if (err) {
    exit(SERVER_ERROR);
  }

  err = 0;
  printf("done\n");

  /*
    From this point all errors can be properly reported to the server, since we
    know the upload location.
   */
  /************************/
  /* Detach device kernel */
  /************************/
  printf("Detaching device kernel... ");
  err = detach_device_kernel(device_vendor_id, device_product_id);
  if (err) {
    sprintf(error_buffer, 
	    "failed to detach device with vendor ID %d and product ID %d.",
	    device_vendor_id,
	    device_product_id);

    err = pack_error(error_buffer, READINGS_FILE);
    if (err) {
      return err;
    }
    http_response_code = http_PUT_JSON(server_url_reading_upload,
				       COOKIE_FILE,
				       NULL, 
				       READINGS_FILE,
				       &http_response_buffer);
    puts(error_buffer);
    exit(USB_OPEN_ERROR);
  }
  printf("done\n");

  /***************/
  /* Open device */
  /***************/
  printf("Opening device... ");
  err = ftdi_usb_open(ftHandle, device_vendor_id, device_product_id);
  if (err) { 
    sprintf(error_buffer,
	    "failed to open device with vendor ID %d and product ID %D.",
	    device_vendor_id,
	    device_product_id);
    
    err = pack_error(error_buffer, READINGS_FILE);
    if (err) {
      exit(IO_ERROR);
    }
    http_response_code = http_PUT_JSON(server_url_reading_upload,
				       COOKIE_FILE,
				       NULL,
				       READINGS_FILE,
				       &http_response_buffer);
    puts(error_buffer);
    exit(USB_OPEN_ERROR);
  }
  printf("done\n");

  /******************/
  /* Prepare device */
  /******************/
  printf("Preparing device... ");
  err = prepare_device(ftHandle);
  if (err) { 
    sprintf(error_buffer,
	    "failed to prepare device with vendor ID %d and product ID %D.",
	    device_vendor_id,
	    device_product_id);
    
    err = pack_error(error_buffer, READINGS_FILE);
    if (err) {
      exit(IO_ERROR);
    }

    http_response_code = http_PUT_JSON(server_url_reading_upload,
				       COOKIE_FILE,
				       NULL,
				       READINGS_FILE,
				       &http_response_buffer);
    puts(error_buffer);
    exit(USB_OPEN_ERROR);
  }
  printf("done\n");

  /*************************/
  /* read data from SEM710 */
  /*************************/
  printf("Reading device... ");
  read_bytes = read_device(ftHandle, 
			   SEM_COMMANDS_cREAD_PROCESS, 
			   reading_buffer);
  ftdi_usb_close(ftHandle);
  
  if (read_bytes <= 0) {
    sprintf(error_buffer,
	    "failed to read device with vendor ID %d and product ID %D.",
	    device_vendor_id,
	    device_product_id);
    
    err = pack_error(error_buffer, READINGS_FILE);
    if (err) {
      exit(IO_ERROR);
    }
    http_response_code = http_PUT_JSON(server_url_reading_upload,
				       COOKIE_FILE,
				       NULL,
				       READINGS_FILE,
				       &http_response_buffer);
    puts(error_buffer);
    exit(USB_READ_ERROR);
  }
  
  get_readings(&readings, reading_buffer, read_bytes);
  printf("done\n");

  /*************************/
  /* Upload data to server */
  /*************************/
  printf("Uploading reading to server... ");
  if (time_until_update <= 0) {
    /*
      if the time since the last read is greater than the required wait time,
      or if the status has changed since the last read, update.
    */
    err = pack_readings(&readings, READINGS_FILE);

    if (err) {
      sprintf(error_buffer,
	      "Failed to write reading to file.");
      
      err = pack_error(error_buffer, READINGS_FILE);
      if (err) {
	exit(IO_ERROR);
      }
      http_response_code = http_PUT_JSON(server_url_reading_upload,
					 COOKIE_FILE,
					 NULL,
					 READINGS_FILE,
					 &http_response_buffer);
      puts(error_buffer);
      exit(IO_ERROR);
    }
    http_PUT_JSON(server_url_reading_upload, 
	     COOKIE_FILE,
	     NULL,
	     READINGS_FILE, 
	     &http_response_buffer);
  }
  printf("done\n");
  /****************/
  /* close device */
  /****************/
  ftdi_deinit(ftHandle);
  ftdi_free(ftHandle);
  
  free(container_num);
  free(last_read_status);

  free(server_login_email);
  free(server_login_password);
  
  free(server_path_authentication);
  free(server_path_containers);
  free(server_path_specifications);
  free(server_path_reading_upload);

  free(certificate_authority_path);

  free(server_url_base);
  free(server_url_authentication);
  free(server_url_specifications);
  free(server_url_reading_upload);

  free(postfield_buffer);
  free(error_buffer);

  deinit_string(&http_response_buffer);
  deinit_string(&http_response_jsobj);
  
  return 0;
}

#endif
