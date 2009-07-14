cmake_minimum_required (VERSION 2.7.20090711)
project(ExportImport C CXX)

# Wipe out the install tree to make sure the exporter works.
add_custom_command(
  OUTPUT ${ExportImport_BINARY_DIR}/CleanupProject
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${ExportImport_BINARY_DIR}/Root
  )
add_custom_target(CleanupTarget ALL DEPENDS ${ExportImport_BINARY_DIR}/CleanupProject)
set_property(
  SOURCE ${ExportImport_BINARY_DIR}/CleanupProject
  PROPERTY SYMBOLIC 1
  )

if(CMAKE_CONFIGURATION_TYPES)
  set(NESTED_CONFIG_TYPE -C "${CMAKE_CFG_INTDIR}")
else(CMAKE_CONFIGURATION_TYPES)
  if(CMAKE_BUILD_TYPE)
    set(NESTED_CONFIG_TYPE -C "${CMAKE_BUILD_TYPE}")
  else(CMAKE_BUILD_TYPE)
    set(NESTED_CONFIG_TYPE)
  endif(CMAKE_BUILD_TYPE)
endif(CMAKE_CONFIGURATION_TYPES)

configure_file(${ExportImport_SOURCE_DIR}/InitialCache.cmake.in
               ${ExportImport_BINARY_DIR}/InitialCache.cmake @ONLY)

# Build and install the exporter.
add_custom_command(
  OUTPUT ${ExportImport_BINARY_DIR}/ExportProject
  COMMAND ${CMAKE_CTEST_COMMAND} ${NESTED_CONFIG_TYPE}
    --build-and-test
    ${ExportImport_SOURCE_DIR}/Export
    ${ExportImport_BINARY_DIR}/Export
    --build-noclean
    --build-project Export
    --build-target install
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-options -C${ExportImport_BINARY_DIR}/InitialCache.cmake
    )
add_custom_target(ExportTarget ALL DEPENDS ${ExportImport_BINARY_DIR}/ExportProject)
add_dependencies(ExportTarget CleanupTarget)
set_property(
  SOURCE ${ExportImport_BINARY_DIR}/ExportProject
  PROPERTY SYMBOLIC 1
  )

# Build and install the importer.
add_custom_command(
  OUTPUT ${ExportImport_BINARY_DIR}/ImportProject
  COMMAND ${CMAKE_CTEST_COMMAND} ${NESTED_CONFIG_TYPE}
   --build-and-test
   ${ExportImport_SOURCE_DIR}/Import
   ${ExportImport_BINARY_DIR}/Import
   --build-noclean
   --build-project Import
   --build-generator ${CMAKE_GENERATOR}
   --build-makeprogram ${CMAKE_MAKE_PROGRAM}
   --build-options -C${ExportImport_BINARY_DIR}/InitialCache.cmake
   )
add_custom_target(ImportTarget ALL DEPENDS ${ExportImport_BINARY_DIR}/ImportProject)
add_dependencies(ImportTarget ExportTarget)
set_property(
  SOURCE ${ExportImport_BINARY_DIR}/ImportProject
  PROPERTY SYMBOLIC 1
  )

add_executable(ExportImport main.c)
add_dependencies(ExportImport ImportTarget)
