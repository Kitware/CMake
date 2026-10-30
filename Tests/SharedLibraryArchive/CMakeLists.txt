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

add_library(nosla_symlinks SHARED sla.c)
set_target_properties(nosla_symlinks PROPERTIES
  AIX_SHARED_LIBRARY_ARCHIVE OFF
  AIX_SHARED_LIBRARY_SYMLINKS ON
  VERSION 1.2.3
  SOVERSION 1
  )
add_custom_command(TARGET nosla_symlinks POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dnosla=$<TARGET_FILE:nosla_symlinks> -Dname=nosla_symlinks -Dversion=1.2.3 -Dsoversion=1 -P${CMAKE_CURRENT_SOURCE_DIR}/nosla-symlinks-check.cmake
  )

add_executable(UseNoSLA_symlinks use_sla.c)
target_link_libraries(UseNoSLA_symlinks PRIVATE nosla_symlinks)
add_custom_command(TARGET UseNoSLA_symlinks POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dexe=$<TARGET_FILE:UseNoSLA_symlinks> -Dexpected=libnosla_symlinks.so.1 -P${CMAKE_CURRENT_SOURCE_DIR}/nosla-symlinks-link-check.cmake
  )

# Without SOVERSION, dependents link to the VERSION name.
add_library(nosla_version SHARED sla.c)
set_target_properties(nosla_version PROPERTIES
  AIX_SHARED_LIBRARY_ARCHIVE OFF
  AIX_SHARED_LIBRARY_SYMLINKS ON
  VERSION 1.2.3
  )
add_executable(UseNoSLA_version use_sla.c)
target_link_libraries(UseNoSLA_version PRIVATE nosla_version)
add_custom_command(TARGET UseNoSLA_version POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dexe=$<TARGET_FILE:UseNoSLA_version> -Dexpected=libnosla_version.so.1.2.3 -P${CMAKE_CURRENT_SOURCE_DIR}/nosla-symlinks-link-check.cmake
  )

cmake_policy(SET CMP0227 NEW)
add_library(nosla_CMP0227 SHARED sla.c)
set_target_properties(nosla_CMP0227 PROPERTIES
  AIX_SHARED_LIBRARY_ARCHIVE OFF
  VERSION 1.2.3
  SOVERSION 1
  )
add_custom_command(TARGET nosla_CMP0227 POST_BUILD VERBATIM
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -Dnosla=$<TARGET_FILE:nosla_CMP0227> -Dname=nosla_CMP0227 -Dversion=1.2.3 -Dsoversion=1 -P${CMAKE_CURRENT_SOURCE_DIR}/nosla-symlinks-check.cmake
  )
