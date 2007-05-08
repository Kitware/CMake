#ifdef _WIN32
#  define CM_TEST_LIB_IMPORT  __declspec( dllimport )
#endif
CM_TEST_LIB_IMPORT void foo();
int main()
{
  foo();
  return 0;
}
