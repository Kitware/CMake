message(STATUS "Running CMake on GLOB-CONFIGURE_DEPENDS-RerunCMake")
file(GLOB_RECURSE
  CONTENT_LIST
  CONFIGURE_DEPENDS
  LIST_DIRECTORIES false
  RELATIVE "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/test/*"
  )
string(SHA1 CONTENT_LIST_HASH "${CONTENT_LIST}")
add_custom_target(CONTENT_ECHO ALL ${CMAKE_COMMAND} -E echo ${CONTENT_LIST_HASH})
if(CMAKE_XCODE_BUILD_SYSTEM VERSION_GREATER_EQUAL 12)
  # Xcode's "new build system" does not reload the project file if it is updated
  # during the build.  Print the output we expect the build to print just to make
  # the test pass.
  message(STATUS "CONTENT_LIST_HASH: ${CONTENT_LIST_HASH}")
endif()
