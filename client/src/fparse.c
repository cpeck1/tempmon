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

cJSON *get_specifications(char *filename)
{
  cJSON *root;
  cJSON *specifications = NULL;

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
      return specifications;
    }
  }
  return NULL;
}
