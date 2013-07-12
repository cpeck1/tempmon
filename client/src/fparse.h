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

  void get_file(FILE *f, char *buffer);

  int get_specifications(char *filename, char **freezer_num, 
		       char **specifications_url, char **auth_usr,
			 char **auth_pwd);
#ifdef __cplusplus
}
#endif

#endif /* __INC_FPARSE_H */
