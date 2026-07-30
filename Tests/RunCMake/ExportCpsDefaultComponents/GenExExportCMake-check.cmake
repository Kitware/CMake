include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/GenExExportCMake-build")

file(READ "${out_dir}/bar.cmake" bar_cmake)
if(NOT "${bar_cmake}" MATCHES "add_library\\(bar INTERFACE IMPORTED\\)")
  string(APPEND RunCMake_TEST_FAILED
    "Interface library 'bar' was not exported\n")
endif()
if(NOT "${bar_cmake}" MATCHES "set_target_properties\\(bar PROPERTIES[ \n]+INTERFACE_LINK_LIBRARIES \"[\\][\$]<1:foo>\"[ \n]*\\)")
  string(APPEND RunCMake_TEST_FAILED
    "Interface library 'bar' has wrong link libraries\n")
endif()
