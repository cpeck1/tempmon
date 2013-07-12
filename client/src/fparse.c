#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "fparse.h"
#include "cJSON.h"

void get_file(FILE *f, char *buffer)
{
  char line[256];

  if (fgets(line, 256, f)) {
    strcpy(buffer, line);
  }
  while (fgets(line, 256, f)) {
    strcat(buffer, line);
  }
}

int get_specifications(char *filename, char **freezer_num, 
		       char **specifications_url, char **auth_user,
		       char **auth_pwd)
{
  cJSON *root;
  cJSON *specifications = NULL;
  cJSON *ob = NULL;

  int filesize;
  int buffsize = 1024;
  char fbuffer[buffsize];
  FILE *dfile;

  dfile = fopen(filename, "r");

  fseek(dfile, 0L, SEEK_END);
  filesize = ftell(dfile);
  fseek(dfile, 0L, SEEK_SET);
    
  if (filesize < buffsize) {
    get_file(dfile, fbuffer);
    fclose(dfile);

    root = cJSON_Parse(fbuffer);
    specifications = cJSON_GetObjectItem(root, "specifications");
    if (specifications != NULL) {
      ob = cJSON_GetObjectItem(specifications, "freezer_num");
      if (ob != NULL) {
	*freezer_num = ob->valuestring;
      } else { return -1; }

      ob = cJSON_GetObjectItem(specifications, "url");
      if (ob != NULL) {
	*specifications_url = ob->valuestring;
      } else { return -1; }

      ob = cJSON_GetObjectItem(specifications, "user");
      if (ob != NULL) {
	*auth_user = ob->valuestring;
      } else { return -1; }

      ob = cJSON_GetObjectItem(specifications, "pwd");
      if (ob != NULL) {
	*auth_pwd = ob->valuestring;
      } else { return -1; }
    }
    else {
      return -1;
    }
    return 0;
  }
}
