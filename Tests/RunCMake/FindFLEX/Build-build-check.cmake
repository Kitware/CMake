foreach(f IN ITEMS
    scanner.c
    src/nested.c
    include/nested/nested.h
    )
  if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${f}")
    string(APPEND RunCMake_TEST_FAILED "Missing generated file:\n  ${f}\n")
  endif()
endforeach()
