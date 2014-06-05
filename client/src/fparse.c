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

void strip(char *s) {
    char *p2 = s;
    while(*s != '\0') {
    	if(*s != '\t' && *s != '\n') {
    		*p2++ = *s++;
    	} else {
    		++s;
    	}
    }
    *p2 = '\0';
}


char *get_file_variable(char *fname, char *vname, char *buffer)
{
  char line[256];
  char *vptr;
  
  int searching;
  /* open the file with path given by fname */
  FILE *f = fopen(fname, "r");
  
  /* scan the file line-by-line for the line beginning with vname */
  searching = 1;
  while(fgets(line, sizeof(line), f) && searching) {
    /* the line contains the desired variable name */
    if( vptr = strstr(line, vname) ) {
      /* put its value into buffer */
      strncpy(buffer, vptr+strlen(vname)+2, strlen(vptr+strlen(vname)));
      strip(buffer);
      searching = 0;
    } 
  }
  fclose(f);
    
  return vptr;
}


int get_cjson_object_from_file(char *filename, char *obj_name, char **buffer)
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
	sprintf(*buffer, "%s", object->valuestring);
	return 0;
      }
    }
    else {
      fclose(dfile);
    }
  }
  return 1;
}
