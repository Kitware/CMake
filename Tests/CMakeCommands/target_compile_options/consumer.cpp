
#ifdef DO_GNU_TESTS

#  ifdef MY_PRIVATE_DEFINE
#  error Unexpected MY_PRIVATE_DEFINE
#  endif

#  ifndef MY_PUBLIC_DEFINE
#  error Expected MY_PUBLIC_DEFINE
#  endif

#  ifndef MY_INTERFACE_DEFINE
#  error Expected MY_INTERFACE_DEFINE
#  endif

#endif

int main() { return 0; }
