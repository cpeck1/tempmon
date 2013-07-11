#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "fparse.h"
#include "cJSON.h"

void get_file(FILE *f, char *buffer)
{
  char line[256];

  while (fgets(line, 256, f)) {
    strcat(buffer, line);
  }
}

int get_specifications(char *filename, char **freezer_num, 
		       char **specifications_url, char **auth_user,
		       char **auth_pwd)
{
  cJSON *root;
  cJSON *specifications;

  int filesize;
  int buffsize = 1024;
  char *fbuffer[buffsize];
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
    *freezer_num = cJSON_GetObjectItem(specifications,
    				      "freezer_num")->valuestring;
    *specifications_url = cJSON_GetObjectItem(specifications,
    					     "url")->valuestring;
    *auth_user = cJSON_GetObjectItem(specifications, "user")->valuestring;
    *auth_pwd = cJSON_GetObjectItem(specifications, "pwd")->valuestring;

    return 0;
  }

  return -1;
}
