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

add_library(imported SHARED IMPORTED)
get_property(aix_sla TARGET imported PROPERTY AIX_SHARED_LIBRARY_ARCHIVE)
if(aix_sla)
  message(FATAL_ERROR "AIX_SHARED_LIBRARY_ARCHIVE initialized on imported target")
endif()

unset(CMAKE_AIX_SHARED_LIBRARY_ARCHIVE)
cmake_policy(SET CMP0182 NEW)

add_library(sla_CMP0182 SHARED sla.c)
get_property(aix_sla_CMP0182 TARGET sla_CMP0182 PROPERTY AIX_SHARED_LIBRARY_ARCHIVE)
if(aix_sla_CMP0182)
  message(FATAL_ERROR "AIX_SHARED_LIBRARY_ARCHIVE initialized without CMAKE_AIX_SHARED_LIBRARY_ARCHIVE")
endif()
add_custom_command(TARGET sla_CMP0182 POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dsla=$<TARGET_FILE:sla_CMP0182> -Dname=sla_CMP0182 -Dsoversion= -P${CMAKE_CURRENT_SOURCE_DIR}/sla-check.cmake
  )

add_executable(UseSLA_CMP0182 use_sla.c)
target_link_libraries(UseSLA_CMP0182 PRIVATE sla_CMP0182)

add_library(nosla_CMP0182 SHARED sla.c)
set_property(TARGET nosla_CMP0182 PROPERTY AIX_SHARED_LIBRARY_ARCHIVE OFF)
add_custom_command(TARGET nosla_CMP0182 POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dnosla=$<TARGET_FILE:nosla_CMP0182> -Dname=nosla_CMP0182 -P${CMAKE_CURRENT_SOURCE_DIR}/nosla-check.cmake
  )
