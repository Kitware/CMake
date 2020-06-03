file(STRINGS ${RunCMake_TEST_BINARY_DIR}/alias_global.txt alias_global)

set(expected "TRUE(lib-global):TRUE;FALSE(lib-local):FALSE;TRUE(lib):FALSE")
if(NOT alias_global STREQUAL expected)
  set(RunCMake_TEST_FAILED "ALIAS_GLOBAL was:\n [[${alias_global}]]\nbut expected:\n [[${expected}]]")
endif()
