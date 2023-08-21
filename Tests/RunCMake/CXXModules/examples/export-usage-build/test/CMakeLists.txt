cmake_minimum_required(VERSION 3.24)
project(cxx_modules_library NONE)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ac01f462-0f5f-432a-86aa-acef252918a6")

find_package(export_usage REQUIRED)

if (NOT TARGET CXXModules::export_usage)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

if (NOT TARGET CXXModules::export_used)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

if (NOT TARGET CXXModules::export_build)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

if (NOT TARGET CXXModules::export_install)
  message(FATAL_ERROR
    "Missing imported target")
endif ()

if (TARGET CXXModules::export_never)
  message(FATAL_ERROR
    "Extra imported target")
endif ()

function (check_property expected property)
  get_property(actual TARGET CXXModules::export_usage
    PROPERTY "${property}")
  if (NOT actual STREQUAL expected)
    message(SEND_ERROR
      "Mismatch for ${property}:\n  expected: ${expected}\n  actual: ${actual}")
  endif ()
endfunction ()

check_property("/usr/exported;/usr/buildiface" "IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES")
check_property("exported;buildiface" "IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS")
check_property("cxx_std_20;cxx_std_11;cxx_std_14" "IMPORTED_CXX_MODULES_COMPILE_FEATURES")
check_property("${export_interfaces_flag}100;${export_interfaces_flag}200" "IMPORTED_CXX_MODULES_COMPILE_OPTIONS")
check_property("$<COMPILE_ONLY:CXXModules::export_used>;$<COMPILE_ONLY:CXXModules::export_build>" "IMPORTED_CXX_MODULES_LINK_LIBRARIES")

# Extract the export-dependent targets from the export file.
file(STRINGS "${export_usage_DIR}/export_usage-targets.cmake" usage_dependent_targets
  REGEX "foreach._target ")
# Rudimentary argument splitting.
string(REPLACE " " ";" usage_dependent_targets "${usage_dependent_targets}")
# Keep only "target" names.
list(FILTER usage_dependent_targets INCLUDE REGEX "CXXModules::")
# Strip quotes.
string(REPLACE "\"" "" usage_dependent_targets "${usage_dependent_targets}")

if (NOT "CXXModules::export_used" IN_LIST usage_dependent_targets)
  message(SEND_ERROR
    "The main export does not require the 'CXXModules::export_used' target")
endif ()
if (NOT "CXXModules::export_build" IN_LIST usage_dependent_targets)
  message(SEND_ERROR
    "The main export does not require the 'CXXModules::export_build' target")
endif ()
if ("CXXModules::export_install" IN_LIST usage_dependent_targets)
  message(SEND_ERROR
    "The main export requires the 'CXXModules::export_install' target")
endif ()
