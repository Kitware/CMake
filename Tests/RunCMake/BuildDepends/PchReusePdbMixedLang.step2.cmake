file(WRITE "${RunCMake_TEST_BINARY_DIR}/cxx_only.h" [[
struct CxxMarker { int value; int step2; };
#define PCH_STEP 2
]])
