cmake_minimum_required(VERSION 4.3)
project(cxx_modules_config_import NONE)

find_package(export_interface_config REQUIRED)
find_package(export_interface_config_cps CONFIG REQUIRED)

function(test_target target set_name)
  set(check_genex "${ARGV2}")
  if (NOT TARGET ${target})
    message(FATAL_ERROR "Missing imported target")
  endif ()

  get_property(file_sets TARGET ${target}
    PROPERTY INTERFACE_CXX_MODULE_SETS)
  if (NOT file_sets STREQUAL set_name)
    message(FATAL_ERROR
      "Incorrect exported file sets in ${target}:\n  ${file_sets}")
  endif ()

  get_property(file_set_files TARGET ${target}
    PROPERTY CXX_MODULE_SET_${set_name})

  get_property(_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

  # Verify the Debug module file is present in the file set.
  if (NOT file_set_files MATCHES "debug/module\.cxx")
    message(FATAL_ERROR
      "Expected debug module in file set:\n  ${file_set_files}")
  endif ()

  # For CMakeConfig, verify genex wrapping for both configs.
  if (check_genex AND _multi_config)
    if (NOT file_set_files MATCHES [[\$<\$<CONFIG:Debug>:.*debug/module\.cxx]])
      message(FATAL_ERROR
        "Missing Debug genex entry in file set:\n  ${file_set_files}")
    endif ()
    if (NOT file_set_files MATCHES [[\$<\$<CONFIG:Release>:.*release/module\.cxx]])
      message(FATAL_ERROR
        "Missing Release genex entry in file set:\n  ${file_set_files}")
    endif ()
  endif ()

  # Verify IMPORTED_CXX_MODULES_DEBUG points to the Debug module.
  get_property(imported_modules_debug TARGET ${target}
    PROPERTY IMPORTED_CXX_MODULES_DEBUG)
  if (NOT imported_modules_debug MATCHES "importable=.*debug/module\.cxx")
    message(FATAL_ERROR
      "Incorrect Debug module entry in IMPORTED_CXX_MODULES_DEBUG:\n  ${imported_modules_debug}")
  endif ()
endfunction()

test_target(CXXModules::export_interface_config "modules" FALSE)
test_target(export_interface_config_cps::export_interface_config "CXX_MODULES" FALSE)
