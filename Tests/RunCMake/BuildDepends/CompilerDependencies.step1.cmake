file(WRITE "${RunCMake_TEST_BINARY_DIR}/main.h" [[
#define COUNT 1
]])

file(WRITE "${RunCMake_TEST_BINARY_DIR}/main.c" [[
#include "main.h"

int main(void) { return COUNT; }
]])
