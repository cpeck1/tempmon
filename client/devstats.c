#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_FILE "product.ini"
#define ID1 "DEVICE_ID_VENDOR"
#define ID2 "DEVICE_ID_PRODUCT"
#define ID3 "DEVICE_ID_SERIAL"

// get index of first occurrence of chr in string; not present returns -1 
int get_char_index(char *string, char chr)
{
  int index = 0;

  while (string[index] != '\0') {
    if (string[index] == chr) {
      return index;
    }
    index++;
  }
  return -1; // not found
}

// get the ID equal to the string phrase in the given line
// returns 1 if id found, and 0 if file is malformed
int get_id_value(char *line, int *id_val) 
{
  int eq_index = get_char_index(line, '=');
  int eol_index = get_char_index(line, '\n');

  if (eq_index == -1 || eol_index == -1) {
    return 0;
  }

  int diff = eol_index - eq_index;
  char val_str[diff];
  
  int i;
  for (i = 0; i < diff - 1; i++) {
    val_str[i] = line[eq_index+1 + i];
  }
  val_str[diff - 1] = '\0';
  *id_val = atoi(val_str);

  return 1;
}

int get_specified_id (FILE *f, char *id_phrase, int *id_val)
{
  int found = 0;
  
  rewind(f);
  char line[256];
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
  FILE *dfile = fopen(DEVICE_FILE, "r");
  if (!dfile) {
    return 0xE01;
  }

  int found_dev_id = get_specified_id(dfile, ID1, vendor_id);
  int found_vend_id = get_specified_id(dfile, ID2, device_id);

  if (found_dev_id && found_vend_id) {
    return 0;
  }
  else {
    return 0xE02;
  }
}
