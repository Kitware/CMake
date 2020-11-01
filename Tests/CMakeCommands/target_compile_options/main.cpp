
#ifdef DO_GNU_TESTS

#  ifndef MY_PRIVATE_DEFINE
#    error Expected MY_PRIVATE_DEFINE
#  endif

#  ifndef MY_PUBLIC_DEFINE
#    error Expected MY_PUBLIC_DEFINE
#  endif

#  ifndef MY_MUTLI_COMP_PUBLIC_DEFINE
#    error Expected MY_MUTLI_COMP_PUBLIC_DEFINE
#  endif

#  ifdef MY_INTERFACE_DEFINE
#    error Unexpected MY_INTERFACE_DEFINE
#  endif

#endif

#ifdef DO_CLANG_TESTS

#  ifndef MY_PRIVATE_DEFINE
#    error Expected MY_PRIVATE_DEFINE
#  endif

#  ifdef MY_PUBLIC_DEFINE
#    error Unexpected MY_PUBLIC_DEFINE
#  endif

#  ifndef MY_MUTLI_COMP_PUBLIC_DEFINE
#    error Expected MY_MUTLI_COMP_PUBLIC_DEFINE
#  endif

#endif

int main()
{
  return 0;
}
