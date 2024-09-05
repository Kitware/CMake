cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_import_interfaces CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

if (NO_PROPERTIES)
  set(package_name "export_interfaces_no_properties")
elseif (WITH_BMIS)
  set(package_name "export_bmi_and_interfaces")
elseif (EXPORT_COMMAND_SEPDIR)
  set(package_name "export_sepdir")
elseif (INCLUDE_PROPERTIES)
  set(package_name "export_include_directories")
elseif (FROM_NINJA)
  set(package_name "export_from_ninja")
elseif (TRANSITIVE_TARGETS)
  set(package_name "export_transitive_targets")
elseif (TRANSITIVE_MODULES)
  set(package_name "export_transitive_modules")
elseif (WITH_HEADERS)
  set(package_name "export_with_headers")
elseif (BUILD_DATABASE)
  include("${CMAKE_SOURCE_DIR}/../export-build-database-setup.cmake")
  set(package_name "export_build_database")
else ()
  set(package_name "export_interfaces")
endif ()
set(target_name "CXXModules::${package_name}")

find_package("${package_name}" REQUIRED)
if (BUILD_DATABASE)
  # Remove `-isystem` flags for better flag checking consistency.
  set_property(TARGET "CXXModules::${package_name}" "CXXModules::provide_flags"
    PROPERTY SYSTEM 0)
  # Disable debug and runtime flag injection.
  set_property(TARGET "CXXModules::${package_name}"
    PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "")
  set_property(TARGET "CXXModules::${package_name}"
    PROPERTY MSVC_RUNTIME_LIBRARY "")
endif ()

add_executable(use_import_interfaces)
target_sources(use_import_interfaces
  PRIVATE
    use.cxx)
target_compile_features(use_import_interfaces PRIVATE cxx_std_20)
target_link_libraries(use_import_interfaces PRIVATE "${target_name}")
if (BUILD_DATABASE AND
    # Detect Clang targeting MSVC ABI.
    CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  # Link to `libcmt.lib`. Build database tests neuter the runtime library
  # selection to make flag matching easier. Manually perform the "link the
  # runtime library" injected into the object files by the runtime library
  # flag.
  target_link_libraries(use_import_interfaces PRIVATE libcmt)
endif ()

add_test(NAME use_import_interfaces COMMAND use_import_interfaces)
