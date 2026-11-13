file(WRITE "${RunCMake_TEST_BINARY_DIR}/pch.h" [[
struct PchMarker { int value; };
#define PCH_STEP 1
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/creator.c" [[
int creator(void) { return 1; }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/uses_pch.c" [[
int uses_pch(void)
{
  struct PchMarker marker = { PCH_STEP };
  return marker.value;
}
]])
set(main [[
struct MainMarker { int value; };
int creator(void);
int uses_pch(void);
int main(void)
{
  struct MainMarker marker = { uses_pch() + creator() - 1 };
  return marker.value;
}
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/main.c" "${main}")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/main_old.c" "${main}")
