cmake_minimum_required(VERSION 4.4)
project(cxx_modules_install_prep_metadata CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_interfaces STATIC)
target_sources(export_interfaces PUBLIC FILE_SET CXX_MODULES FILES importable.cxx)
target_compile_features(export_interfaces PUBLIC cxx_std_20)

set(install_targets export_interfaces)
set(archive_destination "lib")
if (prep_test)
  set_property(TARGET export_interfaces PROPERTY EXCLUDE_FROM_ALL 1)
  # Exporting a target without modules must not build it for module metadata.
  add_library(no_modules STATIC ../install-prep-never-build.cxx)
  add_library(unrelated STATIC ../install-prep-never-build.cxx)
  list(APPEND install_targets no_modules)
  set(archive_destination "artifacts")
  set(archive_options COMPONENT Artifacts EXCLUDE_FROM_ALL)
  set(source_options COMPONENT Sources EXCLUDE_FROM_ALL)
endif ()

install(TARGETS ${install_targets}
  EXPORT CXXModules
  ARCHIVE DESTINATION "${archive_destination}" ${archive_options}
  FILE_SET CXX_MODULES DESTINATION "lib/cxx/miu" ${source_options})

set(module_options CXX_MODULES_DIRECTORY "cxx-modules")
if (prep_no_module_directory)
  set(module_options)
endif ()
install(EXPORT CXXModules
  DESTINATION "lib/cmake/export_interfaces"
  COMPONENT Metadata
  ${module_options}
  ${prep_rule_options})
