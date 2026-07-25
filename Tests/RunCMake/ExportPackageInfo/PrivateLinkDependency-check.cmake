include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/PrivateLinkDependency-build")

file(READ "${out_dir}/cps/mylib/mylib.cps" content)

# Private link dependency must appear in package requires
expect_array("${content}" 1 "requires" "libdep" "components")
expect_value("${content}" "libdep" "requires" "libdep" "components" 0)

# And in per-config dyld_requires
expect_array("${content}" 1
  "components" "mylib" "configurations" "generic" "dyld_requires")
expect_value("${content}" "libdep:libdep"
  "components" "mylib" "configurations" "generic" "dyld_requires" 0)
