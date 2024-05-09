#ifndef CUSTOM_A_IFACE1
#  error "CUSTOM_A_IFACE1 incorrectly not defined"
#endif

#ifndef CUSTOM_A_IFACE2
#  error "CUSTOM_A_IFACE2 incorrectly not defined"
#endif

#ifndef CUSTOM_A_STATIC1
#  error "CUSTOM_A_STATIC1 incorrectly not defined"
#endif

#ifndef CUSTOM_A_TARGET_NAME_STATIC1
#  error "CUSTOM_A_TARGET_NAME_STATIC1 incorrectly not defined"
#endif

#ifndef CUSTOM_A_TARGET_TYPE_STATIC_LIBRARY
#  error "CUSTOM_A_TARGET_TYPE_STATIC_LIBRARY incorrectly not defined"
#endif

#ifndef CUSTOM_B_IFACE1
#  error "CUSTOM_B_IFACE1 incorrectly not defined"
#endif

#ifndef CUSTOM_B_STATIC1
#  error "CUSTOM_B_STATIC1 incorrectly not defined"
#endif

#ifdef CUSTOM_B_STATIC1_IFACE
#  error "CUSTOM_B_STATIC1_IFACE incorrectly defined"
#endif

#ifndef CUSTOM_U_IFACE1
#  error "CUSTOM_U_IFACE1 incorrectly not defined"
#endif

#ifndef CUSTOM_U_IFACE2
#  error "CUSTOM_U_IFACE2 incorrectly not defined"
#endif

#ifndef CUSTOM_U_STATIC1
#  error "CUSTOM_U_STATIC1 incorrectly not defined"
#endif

#ifndef CUSTOM_U_TARGET_NAME_STATIC1
#  error "CUSTOM_U_TARGET_NAME_STATIC1 incorrectly not defined"
#endif

#ifndef CUSTOM_U_TARGET_TYPE_STATIC_LIBRARY
#  error "CUSTOM_U_TARGET_TYPE_STATIC_LIBRARY incorrectly not defined"
#endif

#ifndef CUSTOM_V_IFACE1
#  error "CUSTOM_V_IFACE1 incorrectly not defined"
#endif

#ifndef CUSTOM_V_STATIC1
#  error "CUSTOM_V_STATIC1 incorrectly not defined"
#endif

int static1(void)
{
  return 0;
}
