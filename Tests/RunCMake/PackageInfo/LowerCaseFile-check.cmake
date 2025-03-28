include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/LowerCaseFile-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/lowercase.cps" content)
expect_value("${content}" "LowerCase" "name")

file(READ "${out_dir}/PreserveCase.cps" content)
expect_value("${content}" "PreserveCase" "name")
