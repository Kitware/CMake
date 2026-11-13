file(WRITE "${RunCMake_TEST_BINARY_DIR}/pch.h" [[
struct CommonMarker { int value; };
#ifdef __cplusplus
#include "cxx_only.h"
#endif
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/cxx_only.h" [[
struct CxxMarker { int value; };
#define PCH_STEP 1
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/creator.c" [[
int creator_c(void) { return 0; }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/creator.cxx" [[
int creator_cxx() { return 0; }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/uses_c.c" [[
int uses_c(void)
{
  struct CommonMarker marker = { 0 };
  return marker.value;
}
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/uses_cxx.cxx" [[
extern "C" int uses_c(void);
int main()
{
  CxxMarker marker = { PCH_STEP };
  return marker.value + uses_c();
}
]])
