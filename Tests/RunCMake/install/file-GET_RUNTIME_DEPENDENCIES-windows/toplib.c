__declspec(dllimport) extern void testlib(void);

__declspec(dllexport) void toplib(void)
{
  testlib();
}
