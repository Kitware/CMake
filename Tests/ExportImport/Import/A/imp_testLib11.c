#ifdef TESTLIB10_INTERFACE_CUSTOM_C
#  error "TESTLIB10_INTERFACE_CUSTOM_C incorrectly defined!"
#endif

#ifdef TESTLIB11_INTERFACE_CUSTOM_C
#  error "TESTLIB11_INTERFACE_CUSTOM_C incorrectly defined!"
#endif

#ifndef TESTLIB11_INTERFACE_CUSTOM_D
#  error "TESTLIB11_INTERFACE_CUSTOM_D incorrectly not defined!"
#endif

#ifndef TESTLIB10_INTERFACE_CUSTOM_L
#  error "TESTLIB10_INTERFACE_CUSTOM_L incorrectly not defined!"
#endif

#ifndef TESTLIB11_INTERFACE_CUSTOM_L
#  error "TESTLIB11_INTERFACE_CUSTOM_L incorrectly not defined!"
#endif

#ifndef TESTLIB11_INTERFACE_CUSTOM_M
#  error "TESTLIB11_INTERFACE_CUSTOM_M incorrectly not defined!"
#endif

int testLib11(void);

int main(void)
{
  return testLib11();
}
