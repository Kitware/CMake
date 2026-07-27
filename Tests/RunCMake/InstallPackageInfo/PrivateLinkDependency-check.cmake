include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/PrivateLinkDependency-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/mylib.cps" content)

# Private link dependency must appear in package requires
expect_array("${content}" 1 "requires" "libdep" "components")
expect_value("${content}" "libdep" "requires" "libdep" "components" 0)

# And in per-config dyld_requires
file(READ "${out_dir}/mylib@generic.cps" content)
expect_array("${content}" 1 "components" "mylib" "dyld_requires")
expect_value("${content}" "libdep:libdep" "components" "mylib" "dyld_requires" 0)
