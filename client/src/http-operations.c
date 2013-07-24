#include <curl/curl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cJSON.h"

#define CERT_PATH "~/workspace/tempmon3/tempmon/client/ssl/rpi@localhost.crt"
#define KEY_PATH "~/workspace/tempmon3/tempmon/client/ssl/rpi@localhost.key"
#define CA_CERT_PATH "/home/cpeck1/workspace/tempmon3/tempmon/client/ssl/tempmonCA.crt"

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  char **response_ptr = (char **) stream;

  *response_ptr = strndup(ptr, (size_t) (size*nmemb));

  return sizeof(*response_ptr);
} 

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  retcode = fread(ptr, size, nmemb, stream);
 
  return retcode;
}

void do_web_put(char *url, char *filename, char *user, char *pwd)
{
  CURL *curl;
  CURLcode res;
  FILE *file;
  int hd;
  struct stat file_info;
  struct curl_slist *slist = NULL;

  hd = open(filename, O_RDONLY);
  fstat(hd, &file_info);
  close(hd);

  file = fopen(filename, "r");

  curl = curl_easy_init();
  if (curl) {
    /* we want to use our own read function */
    /* curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback); */

    slist = curl_slist_append(slist, "Content-Type: application/json");
    /* slist = curl_slist_append(slist, "cpeck1:radar"); */
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    
    /* 
       specify target URL, and note that this URL should include a file name,
       not only a directory
    */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, file);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 2);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERT_PATH);

    curl_easy_setopt(curl, CURLOPT_SSLKEY, KEY_PATH);

    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pwd);
    /* 
       provide the size of the upload, we specifically typecast the value to
       curl_off_t since we must be sure to use the correct data size
    */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, 
		     (curl_off_t) file_info.st_size);

    res = curl_easy_perform(curl);

    if (res && res != CURLE_OK) {
      printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(slist);
    fclose(file);
  }
}

cJSON *get_runtime_specifications(char *url, char *user, char *pwd,
				  float *wait_duration, char **upload_url, 
				  int *product_id, int *vendor_id,
				  float *expected_temperature, 
				  float *safe_temperature_range)
{
  /*
    This is all in one function to prevent a memory leak which happened when
    splitting it up, due to restrictions on passing fixed sized char buffers to
    functions
  */
    
  cJSON *webroot = NULL;
  cJSON *webspecs = NULL;

  cJSON *ob = NULL;

  /* keeps the handle to the curl object */
  CURL *curl = NULL;
  char buffer[1024];
  char *response;

  char json_content[1024];

  int ret;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pwd);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 2);
    curl_easy_setopt(curl, CURLOPT_CAINFO, CA_CERT_PATH);

    /* follow locations specified by the response header */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    /* setting a callback function to return the data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    /* perform the request */
    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK && ret != CURLE_WRITE_ERROR) {
      printf("Error code: %d\n", ret);
      curl_easy_cleanup(curl);
      return NULL;
    }
    /* cleaning all curl stuff */
    curl_easy_cleanup(curl); 
    strcpy(buffer, response);
    free(response);
  }
  if (strlen(buffer) > 0) {
    /*
      The following "fix" was necessitated by cJSON's insistence that you cannot
      fetch object items from roots
    */
    strcpy(json_content, "{\"specifications\":");
    strncat(json_content, buffer, strlen(buffer)-1);
    strcat(json_content, "} }");

    webroot = cJSON_Parse(json_content);
    if (webroot != NULL) {
      webspecs = cJSON_GetObjectItem(webroot, "specifications");
      if (webspecs != NULL) {
	ob = cJSON_GetObjectItem(webspecs, "read_frequency");
	if (ob != NULL) {
	  *wait_duration = ob->valuedouble;
	} else { return NULL; }

	ob = cJSON_GetObjectItem(webspecs, "upload_url_root");
	if (ob != NULL) {
	  *upload_url = ob->valuestring;
	  /* strncpy(*upload_url, ob->valuestring, sizeof(*upload_url)); */
	} else { return NULL; }

	ob = cJSON_GetObjectItem(webspecs, "product_id");
	if (ob != NULL) {
	  *product_id = ob->valueint;
	} else { return NULL; }

	ob = cJSON_GetObjectItem(webspecs, "vendor_id");
	if (ob != NULL) {
	  *vendor_id = ob->valueint;
	} else { return NULL; }

	ob = cJSON_GetObjectItem(webspecs, "expected_temperature");
	if (ob != NULL) {
	  *expected_temperature = ob->valuedouble;
	} else { return NULL; }

	ob = cJSON_GetObjectItem(webspecs, "safe_temperature_range");
	if (ob != NULL) {
	  *safe_temperature_range = ob->valuedouble;
	} else { return NULL; }

	return webroot;
      }
    }
  }
  return NULL;
}
