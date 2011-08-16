

# the test program links against the png lib, so test first whether it exists
if(UNIX  AND  "${CMAKE_GENERATOR}" MATCHES "Makefile")

  # build a library which we can search during the test
  add_library(foo STATIC foo.cpp)

  # configure a FindFoo.cmake so it knows where the library can be found
  configure_file(FindFoo.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FindFoo.cmake @ONLY)

  # now set up the test:
  get_target_property(cmakeExecutable cmake LOCATION)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Makefile.in ${CMAKE_CURRENT_BINARY_DIR}/ConfMakefile @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_BINARY_DIR}/main.cpp COPYONLY)

  add_test(FindPackageModeMakefileTest ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_BINARY_DIR}/ConfMakefile )

endif()
