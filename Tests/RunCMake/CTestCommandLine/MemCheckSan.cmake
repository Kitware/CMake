set(MEMORYCHECK_COMMAND "")
include(CTest)
add_test(
  NAME TestSan
  COMMAND ${CMAKE_COMMAND}
    -P ${CMAKE_CURRENT_LIST_DIR}/../ctest_memcheck/test${MEMORYCHECK_TYPE}.cmake
  )
