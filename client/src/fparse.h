#ifndef __INC_FPARSE_H
#define __INC_FPARSE_H

#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif
  /* get index of first occurrence of chr in string; not present returns -1 */

  void get_file(FILE *f, char *buffer);

  cJSON *get_cjson_object_from_file(char *filename, char *obj_name);
#ifdef __cplusplus
}
#endif

#endif /* __INC_FPARSE_H */
