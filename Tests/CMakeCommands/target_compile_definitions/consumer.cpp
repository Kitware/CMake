
#ifdef MY_PRIVATE_DEFINE
#error Unexpected MY_PRIVATE_DEFINE
#endif

#ifndef MY_PUBLIC_DEFINE
#error Expected MY_PUBLIC_DEFINE
#endif

#ifndef MY_INTERFACE_DEFINE
#error Expected MY_INTERFACE_DEFINE
#endif

#ifdef SHOULD_NOT_BE_DEFINED
#error Unexpected SHOULD_NOT_BE_DEFINED
#endif

#ifndef SHOULD_BE_DEFINED
#error Expected SHOULD_BE_DEFINED
#endif

#ifndef DASH_D_DEFINE
#error Expected DASH_D_DEFINE
#endif

int main() { return 0; }
