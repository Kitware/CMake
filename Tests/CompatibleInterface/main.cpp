#include <stdio.h>
#include <string.h>

#ifndef BOOL_PROP1
#  error Expected BOOL_PROP1
#endif

#ifndef BOOL_PROP2
#  error Expected BOOL_PROP2
#endif

#ifndef BOOL_PROP3
#  error Expected BOOL_PROP3
#endif

#ifndef STRING_PROP1
#  error Expected STRING_PROP1
#endif

#ifndef STRING_PROP2
#  error Expected STRING_PROP2
#endif

#ifndef STRING_PROP3
#  error Expected STRING_PROP3
#endif

#ifndef STATIC1_BOOL_PROP1
#  error Expected STATIC1_BOOL_PROP1
#endif

#ifndef STATIC1_STRING_PROP1
#  error Expected STATIC1_STRING_PROP1
#endif

#ifndef STATIC1_NUMBER_MAX_PROP3
#  error Expected STATIC1_NUMBER_MAX_PROP3
#endif

#ifndef STATIC1_NUMBER_MIN_PROP5
#  error Expected STATIC1_NUMBER_MIN_PROP5
#endif

#ifdef OBJECT1_BOOL_PROP1
#  error Unexpected OBJECT1_BOOL_PROP1
#endif

#ifdef OBJECT1_STRING_PROP1
#  error Unexpected OBJECT1_STRING_PROP1
#endif

#ifdef OBJECT1_NUMBER_MAX_PROP3
#  error Unexpected OBJECT1_NUMBER_MAX_PROP3
#endif

#ifdef OBJECT1_NUMBER_MIN_PROP5
#  error Unexpected OBJECT1_NUMBER_MIN_PROP5
#endif

#ifdef IFACE3_BOOL_PROP1
#  error Unexpected IFACE3_BOOL_PROP1
#endif

#ifdef IFACE3_STRING_PROP1
#  error Unexpected IFACE3_STRING_PROP1
#endif

#ifdef IFACE3_NUMBER_MAX_PROP3
#  error Unexpected IFACE3_NUMBER_MAX_PROP3
#endif

#ifdef IFACE3_NUMBER_MIN_PROP5
#  error Unexpected IFACE3_NUMBER_MIN_PROP5
#endif

#ifndef STATIC1_BOOL_PROP5
#  error Expected STATIC1_BOOL_PROP5
#endif

#ifndef STATIC1_STRING_PROP4
#  error Expected STATIC1_STRING_PROP4
#endif

#ifndef STATIC1_NUMBER_MIN_PROP6
#  error Expected STATIC1_NUMBER_MIN_PROP6
#endif

#ifndef STATIC1_NUMBER_MAX_PROP4
#  error Expected STATIC1_NUMBER_MAX_PROP4
#endif

#ifndef OBJECT1_BOOL_PROP5
#  error Expected OBJECT1_BOOL_PROP5
#endif

#ifndef OBJECT1_STRING_PROP4
#  error Expected OBJECT1_STRING_PROP4
#endif

#ifndef OBJECT1_NUMBER_MIN_PROP6
#  error Expected OBJECT1_NUMBER_MIN_PROP6
#endif

#ifndef OBJECT1_NUMBER_MAX_PROP4
#  error Expected OBJECT1_NUMBER_MAX_PROP4
#endif

#ifndef IFACE3_BOOL_PROP5
#  error Expected IFACE3_BOOL_PROP5
#endif

#ifndef IFACE3_STRING_PROP4
#  error Expected IFACE3_STRING_PROP4
#endif

#ifndef IFACE3_NUMBER_MIN_PROP6
#  error Expected IFACE3_NUMBER_MIN_PROP6
#endif

#ifndef IFACE3_NUMBER_MAX_PROP4
#  error Expected IFACE3_NUMBER_MAX_PROP4
#endif

template <bool test>
struct CMakeStaticAssert;

template <>
struct CMakeStaticAssert<true>
{
};

enum
{
  NumericMaxTest1 = sizeof(CMakeStaticAssert<NUMBER_MAX_PROP1 == 100>),
  NumericMaxTest2 = sizeof(CMakeStaticAssert<NUMBER_MAX_PROP2 == 250>),
  NumericMaxTest3 = sizeof(CMakeStaticAssert<NUMBER_MAX_PROP3 == 3>),
  NumericMinTest1 = sizeof(CMakeStaticAssert<NUMBER_MIN_PROP1 == 50>),
  NumericMinTest2 = sizeof(CMakeStaticAssert<NUMBER_MIN_PROP2 == 200>),
  NumericMinTest3 = sizeof(CMakeStaticAssert<NUMBER_MIN_PROP3 == 0xA>),
  NumericMinTest4 = sizeof(CMakeStaticAssert<NUMBER_MIN_PROP4 == 0x10>),
  NumericMinTest5 = sizeof(CMakeStaticAssert<NUMBER_MIN_PROP5 == 5>)
};

#include "iface2.h"

int foo();
#ifdef _WIN32
__declspec(dllimport)
#endif
  int bar();

int main(int argc, char** argv)
{
  int result = 0;
  for (int i = 2; i < argc; i += 2) {
    if (strcmp(argv[i - 1], argv[i]) != 0) {
      fprintf(stderr, "Argument %d expected '%s' but got '%s'.\n", i,
              argv[i - 1], argv[i]);
      result = 1;
    }
  }
  Iface2 if2;
  return result + if2.foo() + foo() + bar();
}
