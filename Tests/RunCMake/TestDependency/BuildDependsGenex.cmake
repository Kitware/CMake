project(TestDependencyBuildDependsGenex C)
enable_testing()

# Targets reached only through generator expressions in build dependencies.
# Each records that it was built, so the tests below fail unless building
# test_prep/all built it.
foreach(name IN ITEMS genex_test genex_discover genex_directory)
  add_executable(${name} EXCLUDE_FROM_ALL ../add_test/main.c)
  add_custom_command(TARGET ${name} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E touch
      "${CMAKE_CURRENT_BINARY_DIR}/${name}-built.txt"
    VERBATIM)
endforeach()
add_custom_target(genex_utility
  COMMAND "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/genex_utility-built.txt"
  VERBATIM)

# An entry that evaluates to nothing adds no dependency.  Were it taken
# literally, the tests would not run for want of this file.
set(never "${CMAKE_CURRENT_BINARY_DIR}/never-built.txt")

# A target referenced by an expression is a dependency, and the artifact the
# expression evaluates to is built by it.  An entry evaluating to a target
# name is a target dependency.
add_test(NAME GenexBuildDepends
  COMMAND "${CMAKE_COMMAND}" -E cat
    "${CMAKE_CURRENT_BINARY_DIR}/genex_test-built.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/genex_utility-built.txt"
  BUILD_DEPENDS
    "$<TARGET_FILE:genex_test>"
    "$<$<BOOL:OFF>:${never}>"
    "$<$<BOOL:ON>:genex_utility>")

add_executable(fake_discovery fake_discovery.c)
discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "GENEX.\\1"
  TEST_ARGS "\\1"
  BUILD_DEPENDS
    "$<TARGET_FILE:genex_discover>"
    "$<$<BOOL:OFF>:${never}>")
add_test(NAME GenexDiscoverBuilt
  COMMAND "${CMAKE_COMMAND}" -E cat
    "${CMAKE_CURRENT_BINARY_DIR}/genex_discover-built.txt")

# The directory property is evaluated as a whole.
set_property(DIRECTORY APPEND PROPERTY CMAKE_TEST_BUILD_DEPENDS
  "$<TARGET_FILE:genex_directory>" "$<$<BOOL:OFF>:${never}>")
add_test(NAME GenexDirectoryBuilt
  COMMAND "${CMAKE_COMMAND}" -E cat
    "${CMAKE_CURRENT_BINARY_DIR}/genex_directory-built.txt")
