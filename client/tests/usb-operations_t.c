#ifndef USBOP_TEST
#define USBOP_TEST

void report_error(int testnum, char *test_desc, char *expected, char *got)
{
  printf("Test failed: test %d\n", testnum);
  printf("\t%s\n", test_desc);
  printf("\tExpected: %s\n", expected);
  printf("\tGot: %s\n", got);
}

int usb_op_test(void) 
{
  char xbf[150];

  int test_number;
  int tests_passed;

  char *test_desc;
  char *got = "";

  uint8_t test[280];
  int index;
  int val;
  uint16_t crc;

  test_desc = xbf; /* magic */
  got = xbf;

  test_number = 0;
  tests_passed = 0;

  /************************************/
  test_desc = "Testing make_crc(case 0)";
  /************************************/
  test_number++;
  
  test[0] = SEM710_START_BIT; test[1] = 0; test[2] = SEM_710_END_BIT;
  

}
