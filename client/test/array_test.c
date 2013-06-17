/*
  The following are tests for the file array.c
*/

#include "array.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include <gtest/gtest.h>

TEST(array,float_from_byte) {
  uint8_t flt_bytes[4];

  for (int i = 0; i < 4; i++) { flt_bytes[i] = 0; }
  float flt = float_from_byte_array(flt_bytes, 0);

  ASSERT_FLOAT_EQ(flt, (float) 0.00000);
}

/*
TEST(array,tests)
{
  char xbf[150];

  int i;

  int test_number;
  int tests_passed;

  char *test_desc;
  char *got = "";

  uint8_t flt_bytes[4];
  float flt;

  uint8_t shrt_bytes[2];
  short shrt;

  uint8_t flt_bytes_exp[4];

  test_desc = xbf;
  got = xbf;

  tests_passed = 0;
  test_number = 0;

  // **************************************************
  test_desc = "Testing float_from_byte_array(case 0)";
  // **************************************************
  test_number++;


  // **************************************************
  test_desc = "Testing float_from_byte_array(case 1)";
  // **************************************************
  test_number++;

  flt_bytes[0] = 0x00;
  flt_bytes[1] = 0x00;
  flt_bytes[2] = 0x80;
  flt_bytes[3] = 0x3F;

  flt = float_from_byte_array(flt_bytes, 0);
  if (floats_nearly_equal(flt, (float) 1)) {
    tests_passed++;
  }
  else {
    sprintf(got, "%f", flt);
    report_error(test_number, test_desc, "1.00000", got);
  }


  // ****************************************************
  test_desc = "Testing float_from_byte_array(case inf)";
// ****************************************************
  test_number++;

  flt_bytes[0] = 0x00;
  flt_bytes[1] = 0x05;
  flt_bytes[2] = 0xC3;
  flt_bytes[3] = 0x47;

  flt = float_from_byte_array(flt_bytes, 0);
  if (floats_nearly_equal(flt, (float) 100000)) {
    tests_passed++;
  }
  else {
    sprintf(got, "%f", flt);
    report_error(test_number, test_desc, "100000", got);
  }

  // **************************************************
  test_desc = "Testing short_from_byte_array(case 0)";
  // **************************************************
  test_number++;

  shrt_bytes[0] = 0; shrt_bytes[1] = 0;

  shrt = short_from_byte_array(shrt_bytes, 0);
  if (shrt == 0) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", shrt);
    report_error(test_number, test_desc, "0", got);
  }

  // **************************************************
  test_desc = "Testing short_from_byte_array(case 1)";
  // **************************************************
  test_number++;

  shrt_bytes[0] = 0x01; shrt_bytes[1] = 0x00;

  shrt = short_from_byte_array(shrt_bytes, 0);
  if (shrt == 1) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", shrt);
    report_error(test_number, test_desc, "1", got);
  }

  // ****************************************************
  test_desc = "Testing short_from_byte_array(case inf)";
  // ****************************************************
  test_number++;

  shrt_bytes[0] = 0xFF; shrt_bytes[1] = 0x7F;

  shrt = short_from_byte_array(shrt_bytes, 0);
  if (shrt == 0x7FFF) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", shrt);
    report_error(test_number, test_desc, "32767", got);
  }


  // **************************************************
  test_desc = "Testing float_into_byte_array(case 0)";
  // **************************************************
  test_number++;

  flt = (float) 0;

  float_into_byte_array(flt_bytes, 0, flt);
  flt_bytes_exp[0] = 0; flt_bytes_exp[1] = 0;
  flt_bytes_exp[2] = 0; flt_bytes_exp[3] = 0;

  if (bytes_equal(flt_bytes, 0, flt_bytes_exp, 0, 4)) {
    tests_passed++;
  }
  else {
    sprintf(got, "{%d, %d, %d, %d}", flt_bytes[0], flt_bytes[1],
	                             flt_bytes[2], flt_bytes[3]);
    report_error(test_number, test_desc, "{0, 0, 0, 0}", got);
  }


  // **************************************************
  test_desc = "Testing float_into_byte_array(case 1)";
  // **************************************************
  test_number++;

  flt = (float) 1;

  float_into_byte_array(flt_bytes, 0, flt);
  flt_bytes_exp[0] = 0x00; flt_bytes_exp[1] = 0x00;
  flt_bytes_exp[2] = 0x80; flt_bytes_exp[3] = 0x3F;

  if (bytes_equal(flt_bytes, 0, flt_bytes_exp, 0, 4)) {
    tests_passed++;
  }
  else {
     sprintf(got, "{%d, %d, %d, %d}", flt_bytes[0], flt_bytes[1],
	                              flt_bytes[2], flt_bytes[3]);
     report_error(test_number, test_desc, "{0, 0, 128, 63}", got);
  }


  // ****************************************************
  test_desc = "Testing float_into_byte_array(case inf)";
  // ****************************************************
  test_number++;

  flt = (float) 100000;

  float_into_byte_array(flt_bytes, 0, flt);
  flt_bytes_exp[0] = 0; flt_bytes_exp[1] = 80;
  flt_bytes_exp[2] = 195; flt_bytes_exp[3] = 71;

  if (bytes_equal(flt_bytes, 0, flt_bytes_exp, 0, 4)) {
    tests_passed++;
  }
  else {
     sprintf(got, "{%d, %d, %d, %d}", flt_bytes[0], flt_bytes[1],
	                              flt_bytes[2], flt_bytes[3]);
     report_error(test_number, test_desc, "{0, 80, 195, 71}", got);
  }

  return 0;
}

*/

