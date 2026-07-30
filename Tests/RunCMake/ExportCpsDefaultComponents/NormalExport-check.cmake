include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/NormalExport-build")

file(READ "${out_dir}/cps/bar/bar.cps" content)
expect_value("${content}" "bar" "name")
expect_value("${content}" "interface" "components" "bar" "type")

expect_value("${content}" "bar" "name")
expect_array("${content}"      2 "requires"  "foo" "components")
expect_value("${content}" "foo1" "requires"  "foo" "components" 0)
expect_value("${content}" "foo2" "requires"  "foo" "components" 1)

string(JSON component GET "${content}" "components" "bar")
expect_array("${component}" 2 "requires")
expect_value("${component}" "foo:foo1" "requires" 0)
expect_value("${component}" "foo:foo2" "requires" 1)

file(READ "${out_dir}/bar.cmake" bar_cmake)
if(NOT "${bar_cmake}" MATCHES "add_library\\(bar INTERFACE IMPORTED\\)")
  string(APPEND RunCMake_TEST_FAILED
    "Interface library 'bar' was not exported\n")
endif()
if(NOT "${bar_cmake}" MATCHES "set_target_properties\\(bar PROPERTIES[ \n]+INTERFACE_LINK_LIBRARIES \"foo::foo1;foo::foo2\"[ \n]*\\)")
  string(APPEND RunCMake_TEST_FAILED
    "Interface library 'bar' has wrong link libraries\n")
endif()
