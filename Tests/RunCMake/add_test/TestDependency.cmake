project(TestDependency C)

enable_testing()

add_executable(TestDependencyExe main.c)
add_custom_command(TARGET TestDependencyExe POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyExe-built.txt"
  BYPRODUCTS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyExe-built.txt")

add_executable(TestDependencyGenex main.c)
add_custom_command(TARGET TestDependencyGenex POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyGenex-built.txt"
  BYPRODUCTS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyGenex-built.txt")

add_custom_target(TestDependencyPrereq
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyPrereq-built.txt"
  BYPRODUCTS
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyPrereq-built.txt"
  VERBATIM)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyFile-built.txt"
  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyFile-built.txt"
  VERBATIM)

add_custom_target(
  FileDependsTarget
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyFile-built.txt"
)

add_subdirectory(TestDependencySubdir)

add_test(NAME TargetBuildTest
  COMMAND
    TestDependencyExe
    $<TARGET_FILE:TestDependencyGenex>
  BUILD_DEPENDS
    TestDependencyPrereq
    "${CMAKE_CURRENT_BINARY_DIR}/TestDependencyFile-built.txt")
