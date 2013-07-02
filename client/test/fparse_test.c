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

TEST(fparse, get_id_value_0)
{
  char *test;
  int found;
  int val;

  test = "This_val=blank";
  found = get_id_value(test, &val);

  EXPECT_EQ(found, 0);
}

TEST(fparse, get_id_value_1) 
{
  char *test;
  int found;
  int val;

  test = "\n";
  found = get_id_value(test, &val);

  EXPECT_EQ(found, 0);
}

TEST(fparse, get_id_value_2) 
{
  char *test;
  int val;

  test = "This_val=1\n";
  
  get_id_value(test, &val);

  EXPECT_EQ(val, 1);
}

TEST(fparse, get_id_value_3)
{
  char *test;
  int val;

  test = "This_val=35623\n";
  get_id_value(test, &val);

  EXPECT_EQ(val, 35623);
}

TEST(fparse, get_specified_id_0)
{
  FILE *f;

  int found;
  int val;

  f = fopen(TEST_FILE, "r");

  found = get_specified_id(f, "fake_id", &val);

  EXPECT_EQ(found, 0);
}

TEST(fparse, get_specified_id_1)
{
  FILE *f;

  int found;
  int val;

  f = fopen(TEST_FILE, "r");

  found = get_specified_id(f, "faker_id", &val);

  EXPECT_EQ(found, 0);
}

TEST(fparse, get_specified_id_2)
{
  FILE *f;
  int val;

  f = fopen(TEST_FILE, "r");
  get_specified_id(f, "real_id", &val);

  EXPECT_EQ(val, 3);
}

TEST(fparse, get_specified_id_3)
{
  FILE *f;

  int val;

  f = fopen(TEST_FILE, "r");
  get_specified_id(f, "realer_id", &val);
  
  EXPECT_EQ(val, 12345);
}
