foreach(f IN ITEMS
    parser.c
    parser.h
    src/nested.c
    include/nested/nested.h
    reports/nested.output
    )
  if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${f}")
    string(APPEND RunCMake_TEST_FAILED "Missing generated file:\n  ${f}\n")
  endif()
endforeach()
