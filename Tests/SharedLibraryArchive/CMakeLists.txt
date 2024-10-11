cmake_minimum_required(VERSION 3.30)
project(SharedLibraryArchive C)

set(CMAKE_AIX_SHARED_LIBRARY_ARCHIVE 1)

add_library(sla SHARED sla.c)
get_property(aix_sla TARGET sla PROPERTY AIX_SHARED_LIBRARY_ARCHIVE)
if(NOT aix_sla)
  message(FATAL_ERROR "AIX_SHARED_LIBRARY_ARCHIVE not initialized on SHARED library")
endif()
add_custom_command(TARGET sla POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dsla=$<TARGET_FILE:sla> -Dname=sla -Dsoversion= -P${CMAKE_CURRENT_SOURCE_DIR}/sla-check.cmake
  )

add_executable(UseSLA use_sla.c)
get_property(aix_sla TARGET UseSLA PROPERTY AIX_SHARED_LIBRARY_ARCHIVE)
if(aix_sla)
  message(FATAL_ERROR "AIX_SHARED_LIBRARY_ARCHIVE initialized on EXECUTABLE")
endif()
target_link_libraries(UseSLA PRIVATE sla)

# Test for versioned names. Everything else should be tested and set above.
add_library(sla_versioned SHARED sla.c)
get_property(aix_sla_versioned TARGET sla_versioned PROPERTY AIX_SHARED_LIBRARY_ARCHIVE)
if(NOT aix_sla_versioned)
  message(FATAL_ERROR "AIX_SHARED_LIBRARY_ARCHIVE not initialized on SHARED library")
endif()
set_target_properties(sla_versioned PROPERTIES OUTPUT_NAME "sla_versioned" VERSION 3 SOVERSION 2)
add_custom_command(TARGET sla_versioned POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dsla=$<TARGET_FILE:sla_versioned> -Dname=sla_versioned -Dsoversion=2 -P${CMAKE_CURRENT_SOURCE_DIR}/sla-check.cmake
  )

add_executable(UseSLA_versioned use_sla.c)
get_property(aix_sla_versioned TARGET UseSLA_versioned PROPERTY AIX_SHARED_LIBRARY_ARCHIVE)
if(aix_sla_versioned)
  message(FATAL_ERROR "AIX_SHARED_LIBRARY_ARCHIVE initialized on EXECUTABLE")
endif()
target_link_libraries(UseSLA_versioned PRIVATE sla_versioned)
