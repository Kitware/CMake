cmake_minimum_required(VERSION 3.10)
project(JavaModExportImport)
if(NOT DEFINED CMake_TEST_NESTED_MAKE_PROGRAM AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  set(CMake_TEST_NESTED_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM}")
endif()

find_package(Java COMPONENTS Development)

# Wipe out the install tree to make sure the exporter works.
add_custom_command(
  OUTPUT ${JavaModExportImport_BINARY_DIR}/CleanupProject
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${JavaModExportImport_BINARY_DIR}/Root
  )
add_custom_target(CleanupTarget ALL DEPENDS ${JavaModExportImport_BINARY_DIR}/CleanupProject)
set_property(
  SOURCE ${JavaModExportImport_BINARY_DIR}/CleanupProject
  PROPERTY SYMBOLIC 1
  )

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(NESTED_CONFIG_TYPE -C "${CMAKE_CFG_INTDIR}")
else()
  if(CMAKE_BUILD_TYPE)
    set(NESTED_CONFIG_TYPE -C "${CMAKE_BUILD_TYPE}")
  else()
    set(NESTED_CONFIG_TYPE)
  endif()
endif()

configure_file(${JavaModExportImport_SOURCE_DIR}/InitialCache.cmake.in
               ${JavaModExportImport_BINARY_DIR}/InitialCache.cmake @ONLY)

# Build the build exporter.
add_custom_command(
  OUTPUT ${JavaModExportImport_BINARY_DIR}/BuildExportProject
  COMMAND ${CMAKE_CTEST_COMMAND} ${NESTED_CONFIG_TYPE}
    --build-and-test
    ${JavaModExportImport_SOURCE_DIR}/BuildExport
    ${JavaModExportImport_BINARY_DIR}/BuildExport
    --build-noclean
    --build-project BuildExport
    --build-generator ${CMAKE_GENERATOR}
    --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
    --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
    --build-options -C${JavaModExportImport_BINARY_DIR}/InitialCache.cmake
  VERBATIM
  )
add_custom_target(BuildExportTarget ALL DEPENDS ${JavaModExportImport_BINARY_DIR}/BuildExportProject)
add_dependencies(BuildExportTarget CleanupTarget)
set_property(
  SOURCE ${JavaModExportImport_BINARY_DIR}/BuildExportProject
  PROPERTY SYMBOLIC 1
  )

# Build and install the install exporter.
add_custom_command(
  OUTPUT ${JavaModExportImport_BINARY_DIR}/InstallExportProject
  COMMAND ${CMAKE_CTEST_COMMAND} ${NESTED_CONFIG_TYPE}
    --build-and-test
    ${JavaModExportImport_SOURCE_DIR}/InstallExport
    ${JavaModExportImport_BINARY_DIR}/InstallExport
    --build-noclean
    --build-project InstallExport
    --build-target install
    --build-generator ${CMAKE_GENERATOR}
    --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
    --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
    --build-options -C${JavaModExportImport_BINARY_DIR}/InitialCache.cmake
  VERBATIM
  )
add_custom_target(InstallExportTarget ALL DEPENDS ${JavaModExportImport_BINARY_DIR}/InstallExportProject)
add_dependencies(InstallExportTarget CleanupTarget)
set_property(
  SOURCE ${JavaModExportImport_BINARY_DIR}/InstallExportProject
  PROPERTY SYMBOLIC 1
  )

# Build and install the importer.
add_custom_command(
  OUTPUT ${JavaModExportImport_BINARY_DIR}/ImportProject
  COMMAND ${CMAKE_CTEST_COMMAND} ${NESTED_CONFIG_TYPE}
    --build-and-test
    ${JavaModExportImport_SOURCE_DIR}/Import
    ${JavaModExportImport_BINARY_DIR}/Import
    --build-noclean
    --build-project Import
    --build-generator ${CMAKE_GENERATOR}
    --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
    --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
    --build-options
      -C${JavaModExportImport_BINARY_DIR}/InitialCache.cmake
      -DJavaBuildExportTest_DIR:PATH=${JavaModExportImport_BINARY_DIR}/BuildExport
      -DJavaInstallExportTest_DIR:PATH=${JavaModExportImport_BINARY_DIR}/Root/share/cmake
  VERBATIM
  )
add_custom_target(ImportTarget ALL DEPENDS ${JavaModExportImport_BINARY_DIR}/ImportProject)
add_dependencies(ImportTarget BuildExportTarget InstallExportTarget)
set_property(
  SOURCE ${JavaModExportImport_BINARY_DIR}/ImportProject
  PROPERTY SYMBOLIC 1
  )

add_executable(JavaModExportImport main.c)
add_dependencies(JavaModExportImport ImportTarget)
