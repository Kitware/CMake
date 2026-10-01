project(TestDependencyObjectLibrary C)
enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_discovery EXCLUDE_FROM_ALL fake_discovery.c)
add_library(dependency_objects OBJECT EXCLUDE_FROM_ALL fake_discovery.c)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dependency_objects-$<CONFIG>.txt"
  CONTENT "$<TARGET_OBJECTS:dependency_objects>")

add_test(NAME OBJECT.direct COMMAND "${CMAKE_COMMAND}" -E true
  BUILD_DEPENDS dependency_objects)
discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "OBJECT.\\1"
  TEST_ARGS "\\1"
  BUILD_DEPENDS dependency_objects)
