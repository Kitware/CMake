// The flag has no effect on C++ sources.
#if defined(__STDC__)
#  error "__STDC__ is defined in C++."
#endif

extern "C" int other(void)
{
  return 0;
}
