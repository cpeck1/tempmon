/*
  The following are tests for the file array.c
*/

#include "array.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include <gtest/gtest.h>

TEST(array, float_from_byte_0) 
{
  uint8_t flt_bytes[4];

  for (int i = 0; i < 4; i++) { flt_bytes[i] = 0; }
  float flt = float_from_byte_array(flt_bytes, 0);

  ASSERT_FLOAT_EQ(flt, 0.00000);
}

TEST(array, float_from_byte_1) 
{
  uint8_t flt_bytes[4];
  float flt;

  flt_bytes[0] = 0x00;
  flt_bytes[1] = 0x00;
  flt_bytes[2] = 0x80;
  flt_bytes[3] = 0x3F;

  flt = float_from_byte_array(flt_bytes, 0);
  
  ASSERT_FLOAT_EQ(flt, 1.00000);
}

TEST(array, float_from_byte_2) 
{
  uint8_t flt_bytes[4];
  float flt;
  
  flt_bytes[0] = 0x00;
  flt_bytes[1] = 0x00;
  flt_bytes[2] = 0x7A;
  flt_bytes[3] = 0x44;

  flt = float_from_byte_array(flt_bytes, 0);

  ASSERT_FLOAT_EQ(flt, (float) 1000);
}

TEST(array, short_from_byte_0) 
{
  uint8_t shrt_bytes[2];
  short shrt;

  shrt_bytes[0] = 0; shrt_bytes[1] = 0;

  shrt = short_from_byte_array(shrt_bytes, 0);
  
  ASSERT_EQ(shrt, 0);
}

TEST(array, short_from_byte_1) 
{
  uint8_t shrt_bytes[2];
  short shrt;

  shrt_bytes[0] = 1; shrt_bytes[1] = 0;

  shrt = short_from_byte_array(shrt_bytes, 0);
  
  ASSERT_EQ(shrt, 1);
}

TEST(array, short_from_byte_2) 
{
  uint8_t shrt_bytes[2];
  short shrt;

  shrt_bytes[0] = 0xFF; shrt_bytes[1] = 0x7F;

  shrt = short_from_byte_array(shrt_bytes, 0);

  ASSERT_EQ(shrt, 32767);
}

TEST(array, float_into_byte_array_0) 
{
  float flt;
  uint8_t flt_bytes[4];
  uint8_t flt_bytes_exp[4];
  int i;
  
  flt = 0;
  flt_bytes_exp[0] = 0; flt_bytes_exp[1] = 0;
  flt_bytes_exp[2] = 0; flt_bytes_exp[3] = 0;

  float_into_byte_array(flt_bytes, 0, flt);

  for (i = 0; i < 4; i++) {
    ASSERT_EQ(flt_bytes[i], flt_bytes_exp[i]);
  }
}

TEST(array, float_into_byte_array_1) 
{
  float flt;
  uint8_t flt_bytes[4];
  uint8_t flt_bytes_exp[4];
  int i;

  flt = 1.00000;
  flt_bytes_exp[0] = 0; flt_bytes_exp[1] = 0;
  flt_bytes_exp[2] = 0x80; flt_bytes_exp[3] = 0x3F;

  float_into_byte_array(flt_bytes, 0, flt);

  for (i = 0; i < 4; i++) {
    ASSERT_EQ(flt_bytes[i], flt_bytes_exp[i]);
  }
}

TEST(array, float_into_byte_array_2) 
{
  float flt;
  uint8_t flt_bytes[4];
  uint8_t flt_bytes_exp[4];
  int i;
  
  flt = (float) 100000;
  flt_bytes_exp[0] = 0; flt_bytes_exp[1] = 80;
  flt_bytes_exp[2] = 195; flt_bytes_exp[3] = 71;

  float_into_byte_array(flt_bytes, 0, flt);

  for (i = 0; i < 4; i++) {
    ASSERT_EQ(flt_bytes[i], flt_bytes_exp[i]);
  }
}



