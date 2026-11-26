file(WRITE "${RunCMake_TEST_BINARY_DIR}/pch.h" [[
struct PchMarker { int value; };
#define PCH_STEP 2
]])
