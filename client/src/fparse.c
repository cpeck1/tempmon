#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define DEVICE_FILE "product.ini"
#define ID1 "DEVICE_ID_VENDOR"
#define ID2 "DEVICE_ID_PRODUCT"
#define ID3 "DEVICE_ID_SERIAL"

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

int get_id_value(char *line, int *id_val) 
{
/* 
   get the ID equal to the string phrase in the given line
   returns 1 if id found, and 0 if file is malformed 
*/
  int eq_index;
  int eol_index;
  int diff;
  char *val_str;
  int i;

  eq_index = get_char_index(line, '=');
  eol_index = get_char_index(line, '\n');

  if (eq_index == -1 || eol_index == -1) {
    return 0;
  }

  diff = eol_index - eq_index;
  val_str = (char *) malloc(diff);
  
  for (i = 0; i < diff - 1; i++) {
    val_str[i] = line[eq_index+1 + i];
  }
  val_str[diff - 1] = '\0';
  *id_val = atoi(val_str);

  free(val_str);
  return 1;
}

int get_specified_id (FILE *f, char *id_phrase, int *id_val)
{
  int found;
  char line[256];
  
  rewind(f);
  found = 0;
  while (!feof(f)) {
    if (fgets(line, 256, f) != NULL) {
      if (strstr(line, id_phrase)) {
	found = get_id_value(line, id_val);
	break;
      }
    }
  }
  return found;
}

int get_device_ids(int *device_id, int *vendor_id)
{
  FILE *dfile;
  int found_dev_id;
  int found_vend_id;

  dfile = fopen(DEVICE_FILE, "r");
  if (!dfile) {
    printf("Error: missing device file\n");
    return -1;
  }

  found_dev_id = get_specified_id(dfile, ID1, vendor_id);
  found_vend_id = get_specified_id(dfile, ID2, device_id);

  if (found_dev_id && found_vend_id) {
    return 0;
  }
  else {
    printf("Error: bad device file\n");
    return -1;
  }
}
