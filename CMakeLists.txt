PROJECT(CMake)

SUBDIRS(Source Modules Templates)

# Include the standard Dart testing module
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

# add some testing dependencies
IF(BUILD_TESTING)
  IF (DART_ROOT)
    ADD_DEPENDENCIES(Nightly cmaketest cmake)
    ADD_DEPENDENCIES(Experimental cmaketest cmake)
    ADD_DEPENDENCIES(NightlyTest cmaketest cmake)
    ADD_DEPENDENCIES(ExperimentalTest cmaketest cmake)
  ENDIF (DART_ROOT)
ENDIF(BUILD_TESTING)

SET(EXECUTABLE_OUTPUT_PATH ${CMake_BINARY_DIR}/Source CACHE PATH 
  "Where to put the executables for CMake"
)

INCLUDE_REGULAR_EXPRESSION("^cm.*")


