PROJECT(CMake)

SUBDIRS(Source Modules Templates)

# Include the standard Dart testing module
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

SET(EXECUTABLE_OUTPUT_PATH ${CMake_BINARY_DIR}/Source CACHE PATH 
  "Where to put the executables for CMake"
)

INCLUDE_REGULAR_EXPRESSION("^cm.*")


