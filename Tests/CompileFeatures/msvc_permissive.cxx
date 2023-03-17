#if !defined(_MSVC_LANG) || _MSVC_LANG < 202002L
#  error "This source must be compiled with MSVC as C++20 or later."
#endif
// Test a construct that is allowed by MSVC only with 'cl -permissive'.
enum class X
{
  Y = 1
};
int array[X::Y];
