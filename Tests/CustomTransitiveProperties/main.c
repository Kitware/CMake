#include <stdio.h>
#include <string.h>

#ifdef CUSTOM_A_IFACE1
#  error "CUSTOM_A_IFACE1 incorrectly defined"
#endif

#ifdef CUSTOM_A_IFACE2
#  error "CUSTOM_A_IFACE2 incorrectly defined"
#endif

#ifdef CUSTOM_A_STATIC1_IFACE
#  error "CUSTOM_A_STATIC1_IFACE incorrectly defined"
#endif

#ifdef CUSTOM_A_OBJECT1_IFACE
#  error "CUSTOM_A_OBJECT1_IFACE incorrectly defined"
#endif

#ifndef CUSTOM_A_MAIN
#  error "CUSTOM_A_MAIN incorrectly not defined"
#endif

#ifdef CUSTOM_B_IFACE1
#  error "CUSTOM_B_IFACE1 incorrectly defined"
#endif

#ifndef CUSTOM_B_STATIC1_IFACE
#  error "CUSTOM_B_STATIC1_IFACE incorrectly not defined"
#endif

#ifndef CUSTOM_B_MAIN
#  error "CUSTOM_B_MAIN incorrectly not defined"
#endif

#ifdef CUSTOM_C_IFACE1
#  error "CUSTOM_C_IFACE1 incorrectly defined"
#endif

#ifndef CUSTOM_C_OBJECT1_IFACE
#  error "CUSTOM_C_OBJECT1_IFACE incorrectly not defined"
#endif

#ifndef CUSTOM_C_MAIN
#  error "CUSTOM_C_MAIN incorrectly not defined"
#endif

#ifndef CUSTOM_U_IFACE1
#  error "CUSTOM_U_IFACE1 incorrectly not defined"
#endif

#ifndef CUSTOM_U_IFACE2
#  error "CUSTOM_U_IFACE2 incorrectly not defined"
#endif

#ifndef CUSTOM_U_MAIN
#  error "CUSTOM_U_MAIN incorrectly not defined"
#endif

#ifdef CUSTOM_U_STATIC1
#  error "CUSTOM_U_STATIC1 incorrectly defined"
#endif

#ifndef CUSTOM_U_STATIC1_IFACE
#  error "CUSTOM_U_STATIC1_IFACE incorrectly not defined"
#endif

#ifdef CUSTOM_U_OBJECT1
#  error "CUSTOM_U_OBJECT1 incorrectly defined"
#endif

#ifndef CUSTOM_U_OBJECT1_IFACE
#  error "CUSTOM_U_OBJECT1_IFACE incorrectly not defined"
#endif

#ifndef CUSTOM_U_TARGET_NAME_CUSTOMTRANSITIVEPROPERTIES
#  error                                                                      \
    "CUSTOM_U_TARGET_NAME_CUSTOMTRANSITIVEPROPERTIES incorrectly not defined"
#endif

#ifndef CUSTOM_U_TARGET_TYPE_EXECUTABLE
#  error "CUSTOM_U_TARGET_TYPE_EXECUTABLE incorrectly not defined"
#endif

#ifndef CUSTOM_V_IFACE1
#  error "CUSTOM_V_IFACE1 incorrectly not defined"
#endif

#ifndef CUSTOM_V_MAIN
#  error "CUSTOM_V_MAIN incorrectly not defined"
#endif

#ifdef CUSTOM_V_STATIC1
#  error "CUSTOM_V_STATIC1 incorrectly defined"
#endif

#ifndef CUSTOM_V_STATIC1_IFACE
#  error "CUSTOM_V_STATIC1_IFACE incorrectly not defined"
#endif

#ifndef CUSTOM_W_IFACE1
#  error "CUSTOM_W_IFACE1 incorrectly not defined"
#endif

#ifndef CUSTOM_W_MAIN
#  error "CUSTOM_W_MAIN incorrectly not defined"
#endif

#ifdef CUSTOM_W_OBJECT1
#  error "CUSTOM_W_OBJECT1 incorrectly defined"
#endif

#ifndef CUSTOM_W_OBJECT1_IFACE
#  error "CUSTOM_W_OBJECT1_IFACE incorrectly not defined"
#endif

extern int static1(void);
extern int object1(void);

int check_args(int argc, char** argv)
{
  int result = 0;
  int i;
  for (i = 2; i < argc; i += 2) {
    if (strcmp(argv[i - 1], argv[i]) != 0) {
      fprintf(stderr, "Argument %d expected '%s' but got '%s'.\n", i, argv[i],
              argv[i - 1]);
      result = 1;
    }
  }
  return result;
}

int main(int argc, char** argv)
{
  return static1() + object1() + check_args(argc, argv);
}
