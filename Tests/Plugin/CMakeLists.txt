cmake_minimum_required (VERSION 2.6)
project(Plugin)

# Test per-target output directory properties.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/plugin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/static)

# We need the dynamic loader support from KWSys to load the plugin in
# the executable.
set(KWSYS_NAMESPACE kwsys)
set(KWSYS_HEADER_ROOT ${Plugin_BINARY_DIR}/include)
set(KWSYS_USE_DynamicLoader 1)
add_subdirectory(${Plugin_SOURCE_DIR}/../../Source/kwsys src/kwsys)

# Configure the location of plugins.
configure_file(${Plugin_SOURCE_DIR}/src/example_exe.h.in
               ${Plugin_BINARY_DIR}/include/example_exe.h @ONLY)

# We need to include headers from the source tree and configured
# headers in the build tree.
include_directories(
  ${Plugin_BINARY_DIR}/include
  ${Plugin_SOURCE_DIR}/include
  )

# Create an executable that exports an API for use by plugins.
add_executable(example_exe src/example_exe.cxx)
set_target_properties(example_exe PROPERTIES
  ENABLE_EXPORTS 1
  OUTPUT_NAME example
  # Test placing exe import library in unique directory.
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/exe
  )
target_link_libraries(example_exe kwsys)

# Create a plugin that uses the API provided by the executable.
# This module "links" to the executable to use the symbols.
add_library(example_mod_1 MODULE src/example_mod_1.c)
target_link_libraries(example_mod_1 example_exe)


if(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG AND
    "${CMAKE_C_CREATE_SHARED_MODULE}" MATCHES "SONAME_FLAG")
  # Add a second plugin that should not have any soname.
  add_library(example_mod_2 MODULE src/example_mod_1.c)
  target_link_libraries(example_mod_2 example_exe)
  set_property(TARGET example_mod_2 PROPERTY NO_SONAME 1)

  # Verify that targets export with proper IMPORTED SONAME properties.
  export(TARGETS example_mod_1 example_mod_2 NAMESPACE exp_
    FILE ${CMAKE_CURRENT_BINARY_DIR}/mods.cmake)
  include(${CMAKE_CURRENT_BINARY_DIR}/mods.cmake)
  get_property(configs TARGET exp_example_mod_1 PROPERTY IMPORTED_CONFIGURATIONS)
  foreach(c ${configs})
    string(TOUPPER "${c}" CONFIG)
    get_property(soname1 TARGET exp_example_mod_1 PROPERTY IMPORTED_SONAME_${CONFIG})
    get_property(soname2 TARGET exp_example_mod_2 PROPERTY IMPORTED_NO_SONAME_${CONFIG})
    if(soname1)
      message(STATUS "exp_example_mod_1 has IMPORTED_SONAME_${CONFIG} as expected: ${soname1}")
    else()
      message(SEND_ERROR "exp_example_mod_1 does not have IMPORTED_SONAME_${CONFIG} but should")
    endif()
    if(soname2)
      message(STATUS "exp_example_mod_2 has IMPORTED_NO_SONAME_${CONFIG} as expected: ${soname2}")
    else()
      message(SEND_ERROR "exp_example_mod_2 does not have IMPORTED_NO_SONAME_${CONFIG} but should")
    endif()
  endforeach()

  # Parse the binary to check for SONAME if possible.
  if("${CMAKE_EXECUTABLE_FORMAT}" MATCHES "ELF")
    find_program(READELF_EXE readelf)
    if(READELF_EXE)
      add_custom_target(check_mod_soname ALL COMMAND
        ${CMAKE_COMMAND} -Dreadelf=${READELF_EXE}
                         -Dmod1=$<TARGET_FILE:example_mod_1>
                         -Dmod2=$<TARGET_FILE:example_mod_2>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/check_mod_soname.cmake
        )
      add_dependencies(check_mod_soname example_mod_1 example_mod_2)
    endif()
  endif()
endif()

# TODO:
#  - create a plugin that links to a static lib
#  - create a plugin that links to a shared lib
