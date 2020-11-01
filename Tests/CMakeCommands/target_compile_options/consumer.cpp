
#ifdef DO_GNU_TESTS

#  ifdef MY_PRIVATE_DEFINE
#    error Unexpected MY_PRIVATE_DEFINE
#  endif

#  ifndef MY_PUBLIC_DEFINE
#    error Expected MY_PUBLIC_DEFINE
#  endif

#  ifndef MY_INTERFACE_DEFINE
#    error Expected MY_INTERFACE_DEFINE
#  endif

#  ifndef MY_MULTI_COMP_INTERFACE_DEFINE
#    error Expected MY_MULTI_COMP_INTERFACE_DEFINE
#  endif

#  ifndef MY_MUTLI_COMP_PUBLIC_DEFINE
#    error Expected MY_MUTLI_COMP_PUBLIC_DEFINE
#  endif

#endif

#ifdef DO_CLANG_TESTS

#  ifdef MY_PRIVATE_DEFINE
#    error Unexpected MY_PRIVATE_DEFINE
#  endif

#  ifndef MY_MULTI_COMP_INTERFACE_DEFINE
#    error Expected MY_MULTI_COMP_INTERFACE_DEFINE
#  endif

#  ifndef MY_MUTLI_COMP_PUBLIC_DEFINE
#    error Expected MY_MUTLI_COMP_PUBLIC_DEFINE
#  endif

#endif

#ifndef CONSUMER_LANG_CXX
#  error Expected CONSUMER_LANG_CXX
#endif

#ifdef CONSUMER_LANG_C
#  error Unexpected CONSUMER_LANG_C
#endif

#if !LANG_IS_CXX
#  error Expected LANG_IS_CXX
#endif

#if LANG_IS_C
#  error Unexpected LANG_IS_C
#endif

int main()
{
  return 0;
}
