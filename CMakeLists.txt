PROJECT(CMake)

SUBDIRS(Source Modules Templates Utilities)

# Include the standard Dart testing module
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)
INCLUDE (${CMAKE_BINARY_DIR}/Source/InitialConfigureFlags.cmake OPTIONAL)
IF(CMAKE_CONFIGURE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_CONFIGURE_INSTALL_PREFIX} )
ENDIF(CMAKE_CONFIGURE_INSTALL_PREFIX)

# use the ansi CXX compile flag for building cmake
IF (CMAKE_ANSI_CXXFLAGS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  
ENDIF (CMAKE_ANSI_CXXFLAGS)

IF (CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")  
ENDIF (CMAKE_ANSI_CFLAGS)

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


