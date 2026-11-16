include(${CMAKE_CURRENT_LIST_DIR}/check-data-dir.cmake)

list(LENGTH build_ids build_count)
list(LENGTH ctest_ids ctest_count)
list(LENGTH install_ids install_count)
if(NOT build_count EQUAL 2 OR NOT ctest_count EQUAL 2 OR
   NOT install_count EQUAL 2)
  add_error("Expected two distinct builds, CTest invocations, and installs, got ${build_count}, ${ctest_count}, and ${install_count}")
endif()
