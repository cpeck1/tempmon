#ifndef __INC_FPARSE_H
#define __INC_FPARSE_H

#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
  /* get index of first occurrence of chr in string; not present returns -1 */
  int get_char_index(char *string, char chr);

  /*
    get the ID equal to the string phrase in the given line
    returns 1 if id found, and 0 if file is malformed
  */
  int get_spec_value(char *line, char **val);

  int get_spec(FILE *f, char *id_phrase, char **val);

  int get_runtime_specifications(int *device_id, int *vendor_id, char **url);

  void get_file(FILE *f, char *buffer);

  int get_specifications(char *filename, char **freezer_num, 
		       char **specifications_url, char **auth_usr,
			 char **auth_pwd);
#ifdef __cplusplus
}
#endif

#endif /* __INC_FPARSE_H */
