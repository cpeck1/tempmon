#include "fparse.h"
#include "cJSON.h"

#include <stdio.h>

#include <math.h>
#include <stdint.h>

#include <gtest/gtest.h>

#define fpath "/home/cpeck1/workspace/tempmon3/tempmon/client/test/testfile.json"

TEST(fparse, fparse_test_0) 
{
  cJSON *root;

  char *fake_dir = "/path/to/fake/file.txt";

  root = get_cjson_object_from_file(fake_dir, "fake object name");
  EXPECT_EQ(NULL, root);
}

TEST(fparse, fparse_test_1) 
{
  cJSON *root;
  cJSON *ob;

  char *test1 = NULL;

  root = get_cjson_object_from_file(fpath, "test");
  if (root != NULL) {
    ob = cJSON_GetObjectItem(root, "test1");
    if (ob != NULL) {
      test1 = ob->valuestring;
    }
  }

  ASSERT_STREQ("1", test1);
  cJSON_Delete(root);
}

TEST(fparse, fparse_test_2) 
{
  cJSON *root;
  cJSON *ob;

  char *test2 = NULL;

  root = get_cjson_object_from_file(fpath, "test");
  if (root != NULL) {
    ob = cJSON_GetObjectItem(root, "test2");
    if (ob != NULL) {
      test2 = ob->valuestring;
    }
  }
  ASSERT_STREQ("2", test2);
  cJSON_Delete(root);
}

TEST(fparse, fparse_test_3) 
{
  cJSON *root;
  cJSON *ob;

  char *test3 = NULL;

  root = get_cjson_object_from_file(fpath, "test");
  if (root != NULL) {
    ob = cJSON_GetObjectItem(root, "test3");
    if (ob != NULL) {
      test3 = ob->valuestring;
    }
  }
  ASSERT_STREQ("3", test3);
  cJSON_Delete(root);
}
