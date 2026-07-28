cmake_minimum_required(VERSION 3.29)
project(cxx_modules_library NONE)

find_package(import_std_not_in_export REQUIRED)

if (NOT TARGET CXXModules::import_std_not_in_export)
  message(FATAL_ERROR
    "Missing CMake config imported target")
endif ()

function (check_property expected property target)
  get_property(actual TARGET ${target}
    PROPERTY "${property}")
  if (NOT DEFINED actual)
    if (NOT expected STREQUAL "<UNDEF>")
      message(SEND_ERROR
        "Mismatch for ${property}:\n  expected: ${expected}\n  actual: NOT-DEFINED")
    endif ()
  elseif (NOT actual STREQUAL expected)
    message(SEND_ERROR
      "Mismatch for ${property}:\n  expected: ${expected}\n  actual: ${actual}")
  endif ()
endfunction ()

check_property("<UNDEF>" "IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES" CXXModules::import_std_not_in_export)
check_property("<UNDEF>" "IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS" CXXModules::import_std_not_in_export)
check_property("cxx_std_23" "IMPORTED_CXX_MODULES_COMPILE_FEATURES" CXXModules::import_std_not_in_export)
check_property("" "IMPORTED_CXX_MODULES_LINK_LIBRARIES" CXXModules::import_std_not_in_export)
check_property("<UNDEF>" "INTERFACE_LINK_LIBRARIES" CXXModules::import_std_not_in_export)

# Extract the export-dependent targets from the export file.
file(STRINGS "${import_std_not_in_export_DIR}/import_std_not_in_export-targets.cmake" usage_dependent_targets
  REGEX "foreach._target ")
# Rudimentary argument splitting.
string(REPLACE " " ";" usage_dependent_targets "${usage_dependent_targets}")
# Remove exported "target" names.
list(FILTER usage_dependent_targets EXCLUDE REGEX "CXXModules::")
# Strip quotes.
string(REPLACE "\"" "" usage_dependent_targets "${usage_dependent_targets}")

if ("@cmake_cxx_std" IN_LIST usage_dependent_targets)
  message(SEND_ERROR
    "The CMake config export requires the '@cmake_cxx_std' target")
endif ()

# CPS import test
find_package(import_std_not_in_export_cps CONFIG REQUIRED)
if (NOT TARGET import_std_not_in_export_cps::import_std_not_in_export)
  message(FATAL_ERROR "Missing CPS imported target")
endif ()

check_property("<UNDEF>" "IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES" import_std_not_in_export_cps::import_std_not_in_export)
check_property("<UNDEF>" "IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS" import_std_not_in_export_cps::import_std_not_in_export)
check_property("<UNDEF>" "IMPORTED_CXX_MODULES_COMPILE_FEATURES" import_std_not_in_export_cps::import_std_not_in_export)
check_property("<UNDEF>" "IMPORTED_CXX_MODULES_LINK_LIBRARIES" import_std_not_in_export_cps::import_std_not_in_export)
check_property("<UNDEF>" "INTERFACE_LINK_LIBRARIES" import_std_not_in_export_cps::import_std_not_in_export)
