include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/PkgConfig-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)
expect_value("${content}" "foo" "name")
expect_array("${content}" 1 "requires" "cps-pkg-test" "components")
expect_value("${content}" "cps-pkg-test" "requires" "cps-pkg-test" "components" 0)
expect_array("${content}" 1 "requires" "cps-pkg-test" "extensions" "cmake" "domains@v1")
expect_value("${content}" "pkg-config" "requires" "cps-pkg-test" "extensions" "cmake" "domains@v1" 0)

foreach(name IN ITEMS foo bar)
  string(JSON component GET "${content}" "components" "${name}")
  expect_array("${component}" 1 "requires")
  expect_value("${component}" "cps-pkg-test:cps-pkg-test" "requires" 0)
endforeach()
