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

  *response_ptr = strndup((const char*) ptr, (size_t) (size*nmemb));

  return sizeof(*response_ptr);
} 

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  retcode = fread(ptr, size, nmemb, (FILE *)stream);
 
  return retcode;
}

void do_web_put(char *url, char *filename, char *ca_path)
{
  /*
    PUT the file with the given filename to the given url, using the given 
    authentication credentials and certificate authority path (this assumes the
    use of self-signed certificates). In the case that ca_path is NULL, 
    this will still attempt to do the PUT anyway (for instance if the server is
    not using SSL authentication). Currently only supports uploading cJSON 
    filetypes.
  */
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
    /* supports uploads of cJSON file types */
    slist = curl_slist_append(slist, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    
    /* specify target URL */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, file);

    if (ca_path != NULL) {
      /* if using SSL */
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 2);
      curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);

      /* no host verification in place as we are using self-signed certs */
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
    }

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

cJSON *get_runtime_specifications(char *url, char *filename,
				  char *ca_path)
{
  /*
    GET from the given url a cJSON object using the given credentials and 
    certificate authority. If ca_path is NULL, proceed with the GET request
    anyway (for instance if the server is not using SSL authentication).

    Generalized http GET caused memory leaks, returning a cJSON object was a
    work-around.
  */
    
  cJSON *webroot = NULL;;

  /* keeps the handle to the curl object */
  CURL *curl = NULL;
  char buffer[1024];
  char *response;
  char json_content[1024];

  int ret;

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
    slist = curl_slist_append(slist, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);


    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    if (ca_path != NULL) {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 2);
      curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
    }
    
    /* specify file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, file);

    /* follow locations specified by the response header */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    /* setting a callback function to return the data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, 
		     (curl_off_t) file_info.st_size);

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
    printf(buffer);
    webroot = cJSON_Parse(buffer);
    if (webroot != NULL) {
      return webroot;
    }
  }
  return NULL;
}
