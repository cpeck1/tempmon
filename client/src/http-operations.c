#include <curl/curl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "cJSON.h"

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  char **response_ptr = (char **) stream;

  *response_ptr = strndup(ptr, (size_t) (size*nmemb));

  return sizeof(*response_ptr);
} 

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  curl_off_t nread;

  retcode = fread(ptr, size, nmemb, stream);
  nread = (curl_off_t)retcode;
 
  return retcode;
}

/* char *do_web_get(char *url, char *buffer[1024], char *user, char *pwd) */
/* { */
/*   /\* keeps the handle to the curl object *\/ */
/*   CURL *curl_handle = NULL; */
/*   /\* to keep the response *\/ */
/*   char *response; */

/*   /\* initializing curl and setting the url *\/ */
/*   curl_handle = curl_easy_init(); */
/*   if (curl_handle) { */
/*     curl_easy_setopt(curl_handle, CURLOPT_URL, url); */
/*     curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1); */

/*     curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user); */
/*     curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pwd); */

/*     /\* follow locations specified by the response header *\/ */
/*     curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1); */

/*     /\* setting a callback function to return the data *\/ */
/*     curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback); */
    
/*     /\* passing the pointer to the response as the callback parameter *\/ */
/*     curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response); */

/*     /\* perform the request *\/ */
/*     curl_easy_perform(curl_handle); */

/*     /\* cleaning all curl stuff *\/ */
/*     curl_easy_cleanup(curl_handle); */

/*     strncpy(*buffer, response, sizeof(response)); */
/*     free(response); */

/*     return 1; */
/*   } */
/*   else { */
/*     return 0; */
/*   } */
/* } */

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
				 int *product_id, int *vendor_id)
{
  cJSON *webroot = NULL;
  cJSON *webspecs = NULL;

  cJSON *ob = NULL;

  /* keeps the handle to the curl object */
  CURL *curl_handle = NULL;
  char buffer[1024];
  char *response;

  char json_content[1024];

  curl_handle = curl_easy_init();
  if (curl_handle) {
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

    curl_easy_setopt(curl_handle, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl_handle, CURLOPT_PASSWORD, pwd);

    /* follow locations specified by the response header */
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

    /* setting a callback function to return the data */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    
    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);

    /* perform the request */
    curl_easy_perform(curl_handle);

    /* cleaning all curl stuff */
    curl_easy_cleanup(curl_handle);
    
    strcpy(buffer, response);
    free(response);
  }
  if (strlen(buffer) > 0) {
    /*
      The following "fix" was necessitated by cJSON's insistence that you cannot
      fetch object items from roots
    */
    strcpy(json_content, "{\"specifications\":");
    strncat(json_content, buffer+1, strlen(buffer)-2);
    strcat(json_content, "}");

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

	return webroot;
      }
    }
  }
  return NULL;
}
