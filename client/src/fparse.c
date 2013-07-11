#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "fparse.h"

#define DEVICE_FILE "product.ini"
#define ID1 "DEVICE_ID_VENDOR"
#define ID2 "DEVICE_ID_PRODUCT"
#define ID3 "URL"

int get_char_index(char *string, char chr)
{
  /* get index of first occurrence of chr in string; not present returns -1 */
  int index;

  index = 0;
  while ((string[index] != '\0') && (index < 256)) {
    if (string[index] == chr) {
      return index;
    }
    index++;
  }
  return -1; /* not found */
}

int get_spec_value(char *line, char **val) 
{
  /* 
     get the ID equal to the string phrase in the given line
     returns 1 if id found, and 0 if file is malformed 
  */
  int eq_index, eol_index, diff, i;
  char valstr[100];
  eq_index = get_char_index(line, '=');
  eol_index = get_char_index(line, '\n');

  if (eq_index == -1 || eol_index == -1) {
    printf("missing eq or eol index\n");
    return 0;
  }

  diff = eol_index - eq_index;
  printf("line=%s\n", line);
  for (i = 0; i < diff - 1; i++) {
    valstr[i] = line[eq_index+1 + i];
  }
  valstr[diff - 1] = '\0';

  *val = valstr;

  return 1;
}

int get_spec(FILE *f, char *phrase, char **val)
{
  int found;
  char line[256];
  
  rewind(f);
  found = 0;
  while (fgets(line, 256, f)) {
    if (strstr(line, phrase)) {
      found = get_spec_value(line, val);
      break;
    }
  }
  return found;
}

int get_runtime_specifications(int *device_id, int *vendor_id, char **url)
{
  FILE *dfile;
  char *vid, *pid, *furl;
  int found_dev_id, found_vend_id, found_url;
  
  dfile = fopen(DEVICE_FILE, "r");
  if (!dfile) {
    printf("Error: missing device file\n");
    return -1;
  }
  found_url = get_spec(dfile, ID3, &furl);
  *url = furl;

  /* found_vend_id = (int) get_spec(dfile, ID1, &vid); */
  /* printf("get spec vid=%s\n", vid); */
  /* *vendor_id = strtol(vid, 0, 16); */
  /* printf("vendor_id1=%d\n", *vendor_id); */

  found_vend_id = get_spec(dfile, ID1, &vid);
  printf("vid=%s\n", vid);
  *vendor_id = strtol(vid, 0, 10);

  found_dev_id = get_spec(dfile, ID2, &pid);
  printf("pid=%s\n", pid);
  *device_id = strtol(pid, 0, 10);

  if (found_dev_id && found_vend_id && found_url) {
    printf("*device_id=%d\n", *device_id);
    printf("*vendor_id=%d\n", *vendor_id);
    printf("*url=%s\n", *url);
    return 0;
  }
  else {
    printf("Error: bad device file\n");
    return -1;
  }
}

void get_file(FILE *f, char *buffer)
{
  char line[256];

  while (fgets(line, 256, f)) {
    strcat(buffer, line);
  }
}
