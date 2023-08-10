
file(WRITE "${RunCMake_TEST_BINARY_DIR}/lib_depends.c" [[

extern void external(void);

void lib_depends(void)
{
  external();
}
]])


file(WRITE "${RunCMake_TEST_BINARY_DIR}/exe_depends.c" [[

extern void external(void);

int main(void)
{
  external();

  return 0;
}
]])
