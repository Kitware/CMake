add_custom_target(TestDependencySubdirPrereq
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencySubdirPrereq-built.txt"
  BYPRODUCTS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencySubdirPrereq-built.txt"
  VERBATIM)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TestDependencySubdirFile-built.txt"
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencySubdirFile-built.txt"
  VERBATIM)

add_custom_target(TestDependencySubdirFile
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/TestDependencySubdirFile-built.txt")

# Duplicate test name with parent directory
add_test(NAME TargetBuildTest
  COMMAND
    "${CMAKE_COMMAND}" -E true
  BUILD_DEPENDS
    TestDependencySubdirPrereq
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencySubdirFile-built.txt")
