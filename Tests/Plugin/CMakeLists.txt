cmake_minimum_required (VERSION 2.6)
PROJECT(Plugin)

# Test per-target output directory properties.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/plugin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/static)

# We need the dynamic loader support from KWSys to load the plugin in
# the executable.
SET(KWSYS_NAMESPACE kwsys)
SET(KWSYS_HEADER_ROOT ${Plugin_BINARY_DIR}/include)
SET(KWSYS_USE_DynamicLoader 1)
ADD_SUBDIRECTORY(${Plugin_SOURCE_DIR}/../../Source/kwsys src/kwsys)

# Configure the location of plugins.
CONFIGURE_FILE(${Plugin_SOURCE_DIR}/src/example_exe.h.in
               ${Plugin_BINARY_DIR}/include/example_exe.h @ONLY)

# We need to include headers from the source tree and configured
# headers in the build tree.
INCLUDE_DIRECTORIES(
  ${Plugin_BINARY_DIR}/include
  ${Plugin_SOURCE_DIR}/include
  )

# Create an executable that exports an API for use by plugins.
ADD_EXECUTABLE(example_exe src/example_exe.cxx)
SET_TARGET_PROPERTIES(example_exe PROPERTIES
  ENABLE_EXPORTS 1
  OUTPUT_NAME example
  # Test placing exe import library in unique directory.
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/exe
  )
TARGET_LINK_LIBRARIES(example_exe kwsys)

# Create a plugin that uses the API provided by the executable.
# This module "links" to the executable to use the symbols.
ADD_LIBRARY(example_mod_1 MODULE src/example_mod_1.c)
TARGET_LINK_LIBRARIES(example_mod_1 example_exe)


IF(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG AND
    "${CMAKE_C_CREATE_SHARED_MODULE}" MATCHES "SONAME_FLAG")
  # Add a second plugin that should not have any soname.
  ADD_LIBRARY(example_mod_2 MODULE src/example_mod_1.c)
  TARGET_LINK_LIBRARIES(example_mod_2 example_exe)
  SET_PROPERTY(TARGET example_mod_2 PROPERTY NO_SONAME 1)

  # Verify that targets export with proper IMPORTED SONAME properties.
  EXPORT(TARGETS example_mod_1 example_mod_2 NAMESPACE exp_
    FILE ${CMAKE_CURRENT_BINARY_DIR}/mods.cmake)
  INCLUDE(${CMAKE_CURRENT_BINARY_DIR}/mods.cmake)
  GET_PROPERTY(configs TARGET exp_example_mod_1 PROPERTY IMPORTED_CONFIGURATIONS)
  FOREACH(c ${configs})
    STRING(TOUPPER "${c}" CONFIG)
    GET_PROPERTY(soname1 TARGET exp_example_mod_1 PROPERTY IMPORTED_SONAME_${CONFIG})
    GET_PROPERTY(soname2 TARGET exp_example_mod_2 PROPERTY IMPORTED_NO_SONAME_${CONFIG})
    IF(soname1)
      MESSAGE(STATUS "exp_example_mod_1 has IMPORTED_SONAME_${CONFIG} as expected: ${soname1}")
    ELSE()
      MESSAGE(SEND_ERROR "exp_example_mod_1 does not have IMPORTED_SONAME_${CONFIG} but should")
    ENDIF()
    IF(soname2)
      MESSAGE(STATUS "exp_example_mod_2 has IMPORTED_NO_SONAME_${CONFIG} as expected: ${soname2}")
    ELSE()
      MESSAGE(SEND_ERROR "exp_example_mod_2 does not have IMPORTED_NO_SONAME_${CONFIG} but should")
    ENDIF()
  ENDFOREACH()

  # Parse the binary to check for SONAME if possible.
  IF("${CMAKE_EXECUTABLE_FORMAT}" MATCHES "ELF")
    FIND_PROGRAM(READELF_EXE readelf)
    IF(READELF_EXE)
      ADD_CUSTOM_TARGET(check_mod_soname ALL COMMAND
        ${CMAKE_COMMAND} -Dreadelf=${READELF_EXE}
                         -Dmod1=$<TARGET_FILE:example_mod_1>
                         -Dmod2=$<TARGET_FILE:example_mod_2>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/check_mod_soname.cmake
        )
      ADD_DEPENDENCIES(check_mod_soname example_mod_1 example_mod_2)
    ENDIF()
  ENDIF()
ENDIF()

# TODO:
#  - create a plugin that links to a static lib
#  - create a plugin that links to a shared lib
