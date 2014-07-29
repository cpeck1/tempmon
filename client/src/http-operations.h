#ifndef __INC_HTTP_OPERATIONS_H
#define __INC_HTTP_OPERATIONS_H

#include <curl/curl.h>
#include <sys/stat.h>

#include "misc-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

char *strcat_percent_encoded(char *destination, char *source);

char *start_postfield(char *destination, char *fieldname, char *fieldvalue);

char *add_postfield(char *destination, char *fieldname, char *fieldvalue);

size_t write_callback(void *ptr, 
		      size_t size, 
		      size_t nmemb, 
		      struct string *s);

size_t read_callback(void *ptr, 
		     size_t size, 
		     size_t nmemb, 
		     void *stream);

/*
  perform a http PUT operation, place the response in the provided buffer and
  return the return code of the operation (i.e. 200, 404, 303 etc) or 0 if
  request failed

  arguments:
    url              - destination URL for the PUT operation
    cookie_file_path - path of the cookie file for the request (if any)
    header           - header of the request
    request_file     - file to PUT
    buffer           - buffer to place response
*/
int http_PUT(char *url,
	     char *cookie_file_path_up,
	     char *cookie_file_down,
	     char *header,
	     char *body,
	     struct string *s);

int http_PUT_JSON(char *url, char *cookie_file_path_up, char *cookie_file_path_down, char *request_file_path, struct string *s);
/*
  perform a http POST operation, place the reposnse in the provided buffer
  and return the return code of the operation (i.e. 200, 404, 303 etc)

  arguments:
    url         - destination URL for the POST operation
    cookie_file_path - path of the cookie file for the request (if any)
    header      - header of the request
    postfields  - body of the request
    buffer      - buffer to place response
*/
int http_POST(char *url,
              char *cookie_file_path_up,
	      char *cookie_file_down,
	      char *header,
	      char *postfields,
	      struct string *s);

/*
  perform a http GET operation, place the response in the provided buffer and
  return the return code of the operation (i.e. 200, 404, 303 etc)

  arguments:
    url              - destination URL for the GET operation
    cookie_file_path - path of the cookie file for the request (if any)
    buffer           - buffer to place the response
*/
int http_GET(char *url,
	     char *cookie_file_path_up,
	     char *cookie_file_down,
	     struct string *s);

/*
  perform a http DELETE operation, place the response in the provided buffer 
  and return the return code of the operation (i.e. 200, 404, 303, etc)

  arguments:
    url              - destination URL for the GET operation
    cookie_file_path - path of the cookies file for the request (if any)
    buffer           - buffer to place the response
*/
int http_DELETE(char *url,
		char *cookie_file,
		struct string *s);

#ifdef __cplusplus
}
#endif

#endif /* __INC_HTTP_OPERATIONS_H */
