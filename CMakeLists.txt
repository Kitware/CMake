PROJECT(CMake)

SUBDIRS(Source Modules Templates)

# Include the standard Dart testing module
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

# use the ansi CXX compile flag for building cmake
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  
# add some testing dependencies
IF(BUILD_TESTING)
  IF (DART_ROOT)
    ADD_DEPENDENCIES(Nightly cmaketest cmake)
    ADD_DEPENDENCIES(Experimental cmaketest cmake)
    ADD_DEPENDENCIES(NightlyTest cmaketest cmake)
    ADD_DEPENDENCIES(ExperimentalTest cmaketest cmake)

    CONFIGURE_FILE(${CMake_SOURCE_DIR}/CMakeLogo.gif ${CMake_BINARY_DIR}/Testing/HTML/TestingResults/Icons/Logo.gif COPYONLY)
  ENDIF (DART_ROOT)
ENDIF(BUILD_TESTING)

SET(EXECUTABLE_OUTPUT_PATH ${CMake_BINARY_DIR}/Source CACHE PATH 
  "Where to put the executables for CMake"
)

INCLUDE_REGULAR_EXPRESSION("^(\\.\\./)?(cm|FLTK|CMake).*")


