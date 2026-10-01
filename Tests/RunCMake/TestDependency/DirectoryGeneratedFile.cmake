project(TestDependencyDirectoryGeneratedFile NONE)
enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

set(output "${CMAKE_CURRENT_BINARY_DIR}/directory-generated.txt")
add_custom_command(OUTPUT "${output}"
  COMMAND "${CMAKE_COMMAND}" -E touch "${output}"
  VERBATIM)
add_custom_target(directory_generated_owner DEPENDS "${output}")
set_property(DIRECTORY PROPERTY CMAKE_TEST_BUILD_DEPENDS "${output}")

add_test(NAME DirectoryGeneratedFileBuilt
  COMMAND "${CMAKE_COMMAND}" -E compare_files "${output}" "${output}")
