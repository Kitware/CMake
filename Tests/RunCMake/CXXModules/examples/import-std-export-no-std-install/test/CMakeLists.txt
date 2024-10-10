cmake_minimum_required(VERSION 3.29)
project(cxx_modules_library NONE)

find_package(import_std_export_no_std REQUIRED)

if (NOT TARGET CXXModules::import_std_export_no_std)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

function (check_property expected property)
  get_property(actual TARGET CXXModules::import_std_export_no_std
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

check_property("<UNDEF>" "IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES")
check_property("<UNDEF>" "IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS")
check_property("cxx_std_23" "IMPORTED_CXX_MODULES_COMPILE_FEATURES")
check_property("" "IMPORTED_CXX_MODULES_LINK_LIBRARIES")
check_property("<UNDEF>" "INTERFACE_LINK_LIBRARIES")
check_property("$<BOOL:>" "CXX_MODULE_STD")

# Extract the export-dependent targets from the export file.
file(STRINGS "${import_std_export_no_std_DIR}/import_std_export_no_std-targets.cmake" usage_dependent_targets
  REGEX "foreach._target ")
# Rudimentary argument splitting.
string(REPLACE " " ";" usage_dependent_targets "${usage_dependent_targets}")
# Remove exported "target" names.
list(FILTER usage_dependent_targets EXCLUDE REGEX "CXXModules::")
# Strip quotes.
string(REPLACE "\"" "" usage_dependent_targets "${usage_dependent_targets}")

if ("__CMAKE::CXX23" IN_LIST usage_dependent_targets)
  message(SEND_ERROR
    "The main export requires the '__CMAKE::CXX23' target")
endif ()
