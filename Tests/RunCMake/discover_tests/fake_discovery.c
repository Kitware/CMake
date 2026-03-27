/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#if defined(_MSC_VER)
#  define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

static int list_tests(int ac, char** av)
{
  (void)ac, (void)av;
  printf("this line should not match\n");
  printf("case_one,LBL1\n");
  printf("\n");
  printf("case_two,LBL2\n");
  printf("garbage,still,has,commas\n");
  printf("case_three,LBL1\n");
  return 0;
}

static int list_fail(int ac, char** av)
{
  (void)ac, (void)av;
  fprintf(stderr, "discovery failure\n");
  return -1;
}

static int list_timeout(int ac, char** av)
{
  (void)ac, (void)av;
  printf("case_one,LBL1\n");
#if defined(_WIN32)
  Sleep(5000);
#else
  sleep(5);
#endif
  return 0;
}

static int list_args(int ac, char** av)
{
  int i;
  for (i = 0; i < ac; i++) {
    printf("case_%s\n", av[i]);
  }
  return 0;
}

static int list_env(int ac, char** av)
{
  (void)ac, (void)av;
  printf("case_%s,%s\n", getenv("TEST_NAME"), getenv("TEST_LABEL"));
  return 0;
}

static int success(int ac, char** av)
{
  (void)ac, (void)av;
  return 0;
}

typedef int (*function_ptr)(int, char**);

struct function_entry
{
  char const* name;
  function_ptr function;
};

struct function_entry const function_map[] = {
  { "--list_tests", list_tests },
  { "--list_fail", list_fail },
  { "--list_timeout", list_timeout },
  { "--list_args", list_args },
  { "--list_env", list_env },
  { "case_one", success },
  { "case_two", success },
  { "case_three", success },
  { NULL, NULL },
};

static function_ptr lookup(char const* name)
{
  struct function_entry const* it = function_map;
  for (; it->name != NULL; ++it) {
    if (strcmp(name, it->name) == 0) {
      return it->function;
    }
  }
  return NULL;
}

int main(int argc, char** argv)
{
  function_ptr fn = argc > 1 ? lookup(argv[1]) : NULL;
  if (fn == NULL) {
    fprintf(stderr, "invalid argument\n");
    return -1;
  }

  return fn(argc - 2, argv + 2);
}
