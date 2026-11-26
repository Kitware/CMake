file(WRITE "${RunCMake_TEST_BINARY_DIR}/pch.h" [[
struct PchMarker { int value; };
#define PCH_STEP 1
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/creator.cxx" [[
int creator() { return 0; }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/uses_pch.cxx" [[
int host_part();
int main()
{
  PchMarker marker = { PCH_STEP };
  return marker.value + host_part();
}
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/host.cu" [[
struct HostMarker { int value; };
int host_part()
{
  HostMarker marker = { 0 };
  return marker.value;
}
]])
