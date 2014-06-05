#ifndef __INC_HTTP_OPERATIONS_H
#define __INC_HTTP_OPERATIONS_H

#include <curl/curl.h>
#include <sys/stat.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t write_callback_func(void *buffer, size_t size,
			   size_t nmemb, void *userp);
size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
char *do_http_put(char *url, char *to_put, char *ca_path);

cJSON *get_runtime_specifications(char *url, char *filename, 
				  char *ca_path);
#ifdef __cplusplus
}
#endif

#endif /* __INC_HTTP_OPERATIONS_H */
