cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_import_interfaces CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

if (NO_PROPERTIES)
  set(package_name "export_interfaces_no_properties")
elseif (WITH_BMIS)
  set(package_name "export_bmi_and_interfaces")
elseif (INCLUDE_PROPERTIES)
  set(package_name "export_include_directories")
elseif (FROM_NINJA)
  set(package_name "export_from_ninja")
elseif (TRANSITIVE_TARGETS)
  set(package_name "export_transitive_targets")
elseif (TRANSITIVE_MODULES)
  set(package_name "export_transitive_modules")
else ()
  set(package_name "export_interfaces")
endif ()
set(target_name "CXXModules::${package_name}")

find_package("${package_name}" REQUIRED)

add_executable(use_import_interfaces)
target_sources(use_import_interfaces
  PRIVATE
    use.cxx)
target_compile_features(use_import_interfaces PRIVATE cxx_std_20)
target_link_libraries(use_import_interfaces PRIVATE "${target_name}")

add_test(NAME use_import_interfaces COMMAND use_import_interfaces)
