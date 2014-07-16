#include <curl/curl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "misc-structs.h"

char *strcat_percent_encoded(char *destination, char *source)
{
  int i;
  int len_source = strlen(source);
  int len_destination;

  for (i = 0; i < len_source; i++) {
    len_destination = strlen(destination);
    switch(source[i]) {
      case '!':
	strcat(destination, "%21");
	break;
      case '#':
        strcat(destination, "%23");
	break;
      case '$':
	strcat(destination, "%24");
	break;
      case '&':
	strcat(destination, "%26");
	break;
      case '\'':
	strcat(destination, "%27");
	break;
      case '(':
	strcat(destination, "%28");
	break;
      case ')':
	strcat(destination, "%29");
	break;
      case '*':
	strcat(destination, "%2A");
	break;
      case '+':
	strcat(destination, "%2B");
	break;
      case ',':
	strcat(destination, "%2C");
	break;
      case '/':
	strcat(destination, "%2F");
	break;
      case ':':
	strcat(destination, "%3A");
	break;
      case ';':
	strcat(destination, "%3B");
	break;
      case '=':
	strcat(destination, "%3D");
	break;
      case '?':
	strcat(destination, "%3F");
	break;
      case '@':
	strcat(destination, "%40");
	break;
      case '[':
	strcat(destination, "%5B");
	break;
      case ']':
	strcat(destination, "%5D");
	break;
      default:
	destination[len_destination] = source[i];
	destination[len_destination+1] = '\0';
    }
  }
  return destination;
}

char *start_postfield(char *destination, char *fieldname, char *fieldvalue)
{
  strcpy(destination, "");
  strcat_percent_encoded(destination, fieldname);
  strcat(destination, "=");
  strcat_percent_encoded(destination, fieldvalue);
  
  return destination;
}

char *add_postfield(char *destination, char *fieldname, char *fieldvalue)
{
  strcat(destination, "&");
  strcat_percent_encoded(destination, fieldname);
  strcat(destination, "=");
  strcat_percent_encoded(destination, fieldvalue);

  return destination;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  retcode = fread(ptr, size, nmemb, (FILE *)stream);
 
  return retcode;
}

int http_PUT(char *url,
	     char *cookie_file_path_up,
	     char *cookie_file_path_down,
	     char *header,
	     char *request_file_path,
	     string *s) 
{
  int ret;
  int hd;
  
  FILE *request_file;
  struct stat file_info;
  
  CURL *curl;
  CURLcode curl_code;
  struct curl_slist *slist = NULL;
  
  /* get request file size */
  if ((hd = open(request_file_path, O_RDONLY)) < 0) {
    /* couldn't get file info, so just return that the operation failed */
    return 0;
  }
  fstat(hd, &file_info);
  close(hd);
  
  /* open request file */
  request_file = fopen(request_file_path, "r");
  if (request_file == NULL) {
    /* couldn't open the file, so return that the operation failed */
    return 0;
  }

  curl = curl_easy_init();
  if (curl) {
    /* Set header to given header */
    slist = curl_slist_append(slist, header);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    
    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* Set operation to PUT */
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    
    /* specify target URL */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    /* provide cookie (up) with request, if given */
    if (cookie_file_path_up != NULL) {
      curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_file_path_up);
    }
    
    /* write all cookies to the cookie path (down), if given */
    if (cookie_file_path_down != NULL) {
      curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_file_path_down);
    }
    
    /* set timeout at 60 seconds */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);

    /* While certificates are self-signed, ignore peer verification */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
    /* setting a callback function to return the data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    
    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

    /* now specify to upload file */
    curl_easy_setopt(curl, CURLOPT_READDATA, (void *) request_file);
    
    /* provide the size of the upload */
    curl_easy_setopt(curl, 
		     CURLOPT_INFILESIZE_LARGE,
		     (curl_off_t) file_info.st_size);

    /* perform the PUT request */
    curl_code = curl_easy_perform(curl);
  }

  /* code is 0 if the operation went through, if so return the http code */
  if (curl_code == 0) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ret);
  } else {
    ret = 0;
  }

  curl_slist_free_all(slist);
  fclose(request_file); 
  curl_easy_cleanup(curl);   
  
  return ret;
}

int http_PUT_JSON(char *url,
		  char *cookie_file_path_up,
		  char *cookie_file_path_down,
		  char *request_file_path,
		  string *s) {
  int ret =  http_PUT(url, 
		      cookie_file_path_up, 
		      cookie_file_path_down, 
		      "Content-Type: application/json", 
		      request_file_path, 
		      s);
  return ret;
  
}

int http_POST(char *url,
              char *cookie_file_path_up,
	      char *cookie_file_path_down,
	      char *header,
	      char *postfields,
	      string *s)
{
  int ret;

  CURL *curl;
  CURLcode curl_code;
  struct curl_slist *slist = NULL;
  
  curl = curl_easy_init();
  
  if(curl) {
    /* specify target URL */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Set header to given header  */
    if (header != NULL) {
      curl_slist_append(slist, header);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    }
    /* provide cookie (up) with request, if given */
    if (cookie_file_path_up != NULL) {
      curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_file_path_up);
    }
    
    /* write all cookies to the cookie path (down), if given */
    if (cookie_file_path_down != NULL) {
      curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_file_path_down);
    }

    /* set the postfields for the request  */
    if (postfields != NULL) {
      /* added a null option here because libcurl IS A DUMMIE */
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
    }

    /* While certificates are self-signed, ignore peer verification */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /* set a callback function to return the data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);

    /* perform the POST request */
    curl_code = curl_easy_perform(curl);
  }
  
  /* code is 0 if the operation succeeded, in which case return the http code */
  if (curl_code == 0) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ret);
  } else {
    ret = 0;
  }
  
  curl_slist_free_all(slist);
  curl_easy_cleanup(curl);
  
  return ret;
}

int http_GET(char *url,
	     char *cookie_file_path_up,
	     char *cookie_file_path_down,
	     string *s)
{
  int ret;
  
  CURL *curl;
  CURLcode curl_code;
  struct curl_slist *slist = NULL;
  
  curl = curl_easy_init();
  if (curl) {
    /* set the URL to what was given by the function callee */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    /* provide cookie (up) with request, if given */
    if (cookie_file_path_up != NULL) {
      curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_file_path_up);
    }
    
    /* write all cookies to the cookie path (down), if given */
    if (cookie_file_path_down != NULL) {
      curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_file_path_down);
    }

    /* While certificates are self-signed, ignore peer verification */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);    
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /* set a callback function to return the data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    
    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
    
    /* perform the GET request */
    curl_code = curl_easy_perform(curl);    
  }
  
  /* code is 0 if the operation succeeded, in which case return the http code */
  if (curl_code == 0) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ret);
  } else {
    ret = 0;
  }
  
  curl_slist_free_all(slist);
  curl_easy_cleanup(curl);
  
  return ret;
}


/* int http_DELETE(char *url, */
/* 		char *cookie_file_path, */
/* 		string *s) */
/* { */
/*   /\* */
/*     Leaving this shell in case this needs to be implemented in the future. */
/*    *\/ */
/*   return 0; */
/* } */

