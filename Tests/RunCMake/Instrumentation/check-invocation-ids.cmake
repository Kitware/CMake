include(${CMAKE_CURRENT_LIST_DIR}/check-data-dir.cmake)

list(LENGTH build_ids build_count)
list(LENGTH ctest_ids ctest_count)
if(NOT build_count EQUAL 2 OR NOT ctest_count EQUAL 2)
  add_error("Expected two distinct builds and two distinct CTest invocations, got ${build_count} and ${ctest_count}")
endif()
