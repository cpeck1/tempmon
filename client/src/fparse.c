#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "fparse.h"
#include "cJSON.h"

void get_file(FILE *f, char *buffer)
{
  /* Puts the entire contents of the given file into the given buffer */
  char line[256];

  if (fgets(line, 256, f)) {
    strcpy(buffer, line);
  }
  while (fgets(line, 256, f)) {
    strcat(buffer, line);
  }
}

cJSON *get_cjson_object_from_file(char *filename, char *obj_name)
{
  /* 
     returns the cJSON object stored as a string with the given name
     in the file with the given filename. 
  */
  cJSON *root;
  cJSON *object = NULL;

  int filesize;
  int buffsize = 1024;
  char fbuffer[buffsize];
  FILE *dfile;

  dfile = fopen(filename, "r");
  if (dfile != NULL) {  /* We found a file with the given filename/path */
    fseek(dfile, 0L, SEEK_END);
    filesize = ftell(dfile);
    fseek(dfile, 0L, SEEK_SET);
    
    if (filesize < buffsize) { 	/* will fit in a buffer of the size reserved */
      get_file(dfile, fbuffer);
      fclose(dfile);

      root = cJSON_Parse(fbuffer);
      object = cJSON_GetObjectItem(root, obj_name);
      if (object != NULL) {
	return object;
      }
    }
    else {
      fclose(dfile);
    }
  }
  return NULL;
}
