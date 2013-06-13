#ifndef DEVSTATS_TEST
#define DEVSTATS_TEST

#define TEST_FILE "testfile.ini"
#include <stdio.h>

void report_error(int testnum, char *test_desc, char *expected, char *got)
{
  printf("Test failed: %d\n", testnum);
  printf("\t%s\n", test_desc);
  printf("\tExpected: %s\n", expected);
  printf("\tGot: %s\n", got);
}

int file_parse_test(void)
{
  int test_number;
  int tests_passed;
  int tests_failed;
  char *test_desc;
  char *got = "";

  int i;
  char *test;
  int index;

  test_number = 0;
  tests_passed = 0;
  tests_failed = 0;

  /******************************************/
  test_desc = "Testing get_char_index(case 0)";
  /******************************************/
  test_number++;

  test = "a";
  index = get_char_index(test, 'b');
  if (index == -1) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", index);
    report_error(test_number, test_desc, "-1", got);
  }  

  test_number++;
  test = "b";
  index = get_char_index(test, 'b');
  if (index == 0) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", index);
    report_error(test_number, test_desc, "0", got);
  }

  /*******************************************/
  test_desc = "Testing get_char_index(case 1)";
  /*******************************************/
  test = "abcdefghijklmnopqrstuvwxyz";

  test_number++;
  index = get_char_index(test, 'z');
  if (index == 25) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", index);
    report_error(test_number, test_desc, "25", got);
  }

  test_number++;
  index = get_char_index(test, 'a');
  if (index == 0) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", index);
    report_error(test_number, test_desc, "0", got);
  }

  test_number++;
  index = get_char_index(test, 'n');
  if (index == 13) {
    tests_passed++;
  }
  else {
    sprintf(got, "%d", index);
    report_error(test_number, test_desc, "13", got);
  }

  return 0;
}

#endif
