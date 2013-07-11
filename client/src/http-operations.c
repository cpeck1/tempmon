#include <curl/curl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cJSON.h"

void write_callback_func(void *buffer, size_t size,
				  size_t nmemb, void *userp)
{
  char **response_ptr =  (char**)userp;

  /* assuming the response is a string */
  *response_ptr = strndup(buffer, (size_t)(size *nmemb));
    
}

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  curl_off_t nread;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */ 
  retcode = fread(ptr, size, nmemb, stream);
 
  nread = (curl_off_t)retcode;
 
  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);
 
  return retcode;
}

char *do_web_get(char *url, char *user, char *pwd)
{
  /* keeps the handle to the curl object */
  CURL *curl_handle = NULL;
  /* to keep the response */
  char *response = NULL;  

  /* initializing curl and setting the url */
  curl_handle = curl_easy_init();
  if (curl_handle) {
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pwd);

    /* follow locations specified by the response header */
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

    /* setting a callback function to return the data */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback_func);
    
    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);

    /* perform the request */
    curl_easy_perform(curl_handle);

    /* cleaning all curl stuff */
    curl_easy_cleanup(curl_handle);

    return response;
  }
  else {
    return "";
  }
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
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

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
  }
}

int get_runtime_specifications(char *url, char *user, char *pwd,
			       float *wait_duration, char **upload_url, 
			       int *product_id, int *vendor_id)
{
  cJSON *webroot;
  cJSON *webspecs;

  int specification_len = 1024;
  char *content;
  char json_content[specification_len];

  content = do_web_get(url, user, pwd);

  if ((int) strlen(content) < specification_len) {
    /*
      The following "fix" was necessitated by cJSON's insistence that you cannot
      fetch object items from roots
    */
    strcpy(json_content, "{\"specifications\":");
    strncat(json_content, content+1, strlen(content)-2);
    strcat(json_content, "}");

    webroot = cJSON_Parse(json_content);
    webspecs = cJSON_GetObjectItem(webroot, "specifications");

    *wait_duration = (float) cJSON_GetObjectItem(webspecs,
						 "read_frequency")->valuedouble;
    *upload_url = cJSON_GetObjectItem(webspecs,
					  "upload_url_root")->valuestring;
    *product_id = cJSON_GetObjectItem(webspecs, "product_id")->valueint;
    *vendor_id = cJSON_GetObjectItem(webspecs, "vendor_id")->valueint;  

    return 0;
  }

  return -1;
}
