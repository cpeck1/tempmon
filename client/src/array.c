#ifndef _ARRAY
#define _ARRAY
#include <stdint.h>

#define L_ENDIAN (get_endianness() == 0)
#define B_ENDIAN (get_endianness() == 1)

int get_endianness()
{
   int a = 0x12345678;
   unsigned char *c = (unsigned char*)(&a);
   if (*c == 0x78) {
      return 0;
   }
   else {
      return 1;
   }
}

float float_from_byte_array(uint8_t *byte_array, int start_index)
{
  int i;
  float f1;
  int size;
  uint8_t *bytes;

  size = (int) sizeof(float);
  bytes = (uint8_t *) malloc(size);

  if (B_ENDIAN) {
    for (i = 0; i < size; i++) {
      bytes[(size-1) - i] = byte_array[start_index + i];
    }
    f1 = *((float *)(&bytes[0]));
  }
  else {
    f1 = *((float *)(&byte_array[start_index]));
  }

  free(bytes);
  return f1;
}

uint16_t short_from_byte_array(uint8_t *byte_array, int start_index)
{
  int i;
  short s1;
  int size;
  uint8_t *bytes;

  size = (int) sizeof(short);
  bytes = (uint8_t *) malloc(size);

  if (B_ENDIAN) {
    for (i = 0; i < size; i++) {
      bytes[(size-1) - i] = byte_array[start_index + i];
    }
    s1 = *((short *)(&bytes[0]));
  }
  else {
    s1 = *((short *)(&byte_array[start_index]));
  }

  free(bytes);
  return s1;
}

int string_from_byte_array(uint8_t *byte_array, char *string, 
			   int start_index, int end_index)
{
  /* 
     transfer the bytes in byte_array from start_index to end_index (inclusive)
     into string; returns the size of the string array (for lack of a better 
     thing to return)
  */
  int i;
  int j;

  j = 0;
  for (i = start_index; i <= end_index; i++) {
    string[j] = (char) byte_array[i];
    j++;
  }
  
  return j;
}

int float_into_byte_array(uint8_t *byte_array, int index, float flt)
{
  /* 
     insert flt into byte_array at index, returning the index of the next open
     space in the array
  */
  int retval;
  int i;

  retval = index + 4;
  if (B_ENDIAN) {
    for (i = 3; i >= 0; i--) {
      byte_array[index + 3 - i] = *(((char *) &flt) + i);
    }
  }
  else {
    for (i = 0; i < 4; i++) {
      byte_array[index + i] = *(((char *) &flt) + i);
    }
  }
  return retval;
}

#endif
