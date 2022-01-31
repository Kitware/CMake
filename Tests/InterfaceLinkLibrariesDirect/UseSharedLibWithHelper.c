extern int testSharedLibHelperObj(int n);

#ifdef testSharedLibHelperObj_NO_OBJECT
int testSharedLibHelperObj(int n)
{
  return n;
}
#endif

#if defined(_WIN32)
__declspec(dllimport)
#endif
  int testSharedLibWithHelper(int n);

int main(void)
{
  return testSharedLibWithHelper(0) + testSharedLibHelperObj(0);
}
