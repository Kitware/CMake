enable_language(C)
enable_testing()

set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "${CMAKE_SOURCE_DIR}/shared/bad-discovery-properties.cmake")
