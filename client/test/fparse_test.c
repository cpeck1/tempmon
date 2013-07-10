#include "fparse.h"

#include <stdio.h>

#include <math.h>
#include <stdint.h>

#include <gtest/gtest.h>

#define TEST_FILE "/home/cpeck1/workspace/tempmon/tempmon/client/tests/testfile.ini"

TEST(fparse, get_char_index_0)
{
  char *test;
  int index;
  
  test = "a";
  index = get_char_index(test, 'b');

  EXPECT_EQ(index, -1);
}

TEST(fparse, get_char_index_1)
{
  char *test;
  int index;
  
  test = "a";
  index = get_char_index(test, 'a');

  EXPECT_EQ(index, 0);
}

TEST(fparse, get_char_index_2)
{
  char *test;
  int index;
  
  test = "abcdefghijklmnopqrstuvwxyz";
  index = get_char_index(test, 'z');

  EXPECT_EQ(index, 25);
}

TEST(fparse, get_char_index_3)
{
  char *test;
  int index;
  
  test = "abcdefghijklmnopqrstuvwxyz";
  index = get_char_index(test, 'a');

  EXPECT_EQ(index, 0);
}

TEST(fparse, get_char_index_4)
{
  char *test;
  int index;
  
  test = "abcdefghijklmnopqrstuvwxyz";
  index = get_char_index(test, 'n');

  EXPECT_EQ(index, 13);
}

TEST(fparse, get_spec_value_0)
{
  char *test;
  int found;
  char *val;

  test = "This_val=blank";
  found = get_spec_value(test, &val);
  EXPECT_EQ(found, 0);
}

TEST(fparse, get_spec_value_1) 
{
  char *test;
  int found;
  char *val;

  test = "\n";
  found = get_spec_value(test, &val);

  EXPECT_EQ(found, 0);
}

TEST(fparse, get_spec_value_2) 
{
  char *test;
  char *val;
  int rval;

  test = "This_val=1\n";
  
  get_spec_value(test, &val);
  rval = atoi(val);
  EXPECT_EQ(rval, 1);
}

TEST(fparse, get_spec_value_3)
{
  char *test;
  char *val;
  int rval;

  test = "This_val=35623\n";
  get_spec_value(test, &val);
  rval = atoi(val);
  EXPECT_EQ(rval, 35623);
}

TEST(fparse, get_spec_0)
{
  FILE *f;

  int found;
  char *val;
  f = fopen(TEST_FILE, "r");

  found = get_spec(f, "fake_id", &val);
  EXPECT_EQ(found, 0);
}

TEST(fparse, get_spec_1)
{
  FILE *f;

  int found;
  char *val;

  f = fopen(TEST_FILE, "r");

  found = get_spec(f, "faker_id", &val);

  EXPECT_EQ(found, 0);
}

TEST(fparse, get_spec_2)
{
  FILE *f;
  char *val;
  int rval;

  f = fopen(TEST_FILE, "r");
  get_spec(f, "real_id", &val);
  rval = atoi(val);

  EXPECT_EQ(rval, 3);
}

TEST(fparse, get_spec_3)
{
  FILE *f;

  char *val;
  int rval;

  f = fopen(TEST_FILE, "r");
  get_spec(f, "realer_id", &val);
  rval = atoi(val);

  EXPECT_EQ(rval, 12345);
}
