
file(WRITE "${RunCMake_TEST_BINARY_DIR}/external.c" [[


#if defined(_WIN32)
__declspec(dllexport)
#endif
  void external(void)
{
}
]])
