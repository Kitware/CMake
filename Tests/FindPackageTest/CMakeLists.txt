PROJECT(FindPackageTest)

# Look for a package that has a find module and may be found.
FIND_PACKAGE(OpenGL QUIET)

# Look for a package that has no find module and will not be found.
FIND_PACKAGE(NotAPackage QUIET)

# Look for a package that has an advanced find module.
FIND_PACKAGE(VTK QUIET)

ADD_EXECUTABLE(FindPackageTest FindPackageTest.cxx)

# test behaviour of cmFindBase wrt. the CMAKE_FIND_PREFIX_PATH variable
# foo.h should be found in ${CMAKE_CURRENT_SOURCE_DIR}/include:

SET(CMAKE_FIND_PREFIX_PATH /blub /blah "${CMAKE_CURRENT_SOURCE_DIR}")
FIND_PATH(FOO_DIR foo.h)

IF(NOT FOO_DIR)
  MESSAGE(FATAL_ERROR "Did not find foo.h which is in ${CMAKE_CURRENT_SOURCE_DIR}/include
  CMAKE_FIND_PREFIX_PATH = ${CMAKE_FIND_PREFIX_PATH}")
ENDIF(NOT FOO_DIR)

