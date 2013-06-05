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
  // bytes are stored little Endian, so reverse first
  int i;
  float f1;
  int size = (int) sizeof(float);
  uint8_t chars[size];

  if (B_ENDIAN) {
    for (i = 0; i < size; i++) {
      chars[(size-1) - i] = byte_array[start_index + i];
    }
    f1 = *((float *)(&chars[0]));
  }
  else {
    f1 = *((float *)(&byte_array[start_index]));
  }
  return f1;
}

uint16_t short_from_byte_array(uint8_t *byte_array, int start_index)
{
  int i;
  short s1;
  int size = (int) sizeof(short);
  uint8_t chars[size];

  // byte_array = {14, 23} 
  // chars[1] = start_index[0]; chars[0] = start_index[1]
  // chars == [23, 14]
  
  if (B_ENDIAN) {
    for (i = 0; i < size; i++) {
      chars[(size-1) - i] = byte_array[start_index + i];
    }
    s1 = *((short *)(&chars[0]));
  }
  else {
    s1 = *((short *)(&byte_array[start_index]));
  }
  return s1;
}

int string_from_byte_array(uint8_t *byte_array, char *string, 
			   int start_index, int end_index)
{
  // transfer the bytes in byte_array from start_index to end_index (inclusive)
  // into string; returns the size of the string array (for lack of a better 
  // thing to return)
  int i;
  int j = 0;

  for (i = start_index; i <= end_index; i++) {
    string[j] = (char) byte_array[i];
    j++;
  }
  
  return j;
}

#endif
