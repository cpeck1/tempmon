#ifndef TEST_FUNCS
#define TEST_FUNCS

void report_error(int testnum, char *test_desc, char *expected, char *got)
{
  printf("Test failed: test %d\n", testnum);
  printf("\t%s\n", test_desc);
  printf("\tExpected: %s\n", expected);
  printf("\tGot: %s\n", got);
}

int floats_nearly_equal(float flt1, float flt2)
{
  /* 
     Added because floating point precision is a little off. Returns whether
     two floats are practically equal.
  */
  if (fabs((flt1 + 0.00001)/flt2) > 0.950) {
    return 1;
  }
  return 0;
}

int bytes_equal(uint8_t *array1, int start1, uint8_t *array2, int start2, 
		int len) {
  while ((abs(start1 - len) > 0) && (abs(start2 - len) > 0)) {
    if (array1[start1] != array2[start2]) {
      return 0;
    }
    start1++;
    start2++;
  }
  return 1;
}

#endif
