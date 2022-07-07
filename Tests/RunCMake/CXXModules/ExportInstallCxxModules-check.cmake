file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Export/eee57a7e91412f1be699e9b63fa9d601/exp.cmake" export_script)

if (NOT export_script MATCHES [[include\("\${CMAKE_CURRENT_LIST_DIR}/cxx-modules/cxx-modules\.cmake"\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find C++ module property script inclusion")
endif ()

file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Export/eee57a7e91412f1be699e9b63fa9d601/cxx-modules/cxx-modules.cmake" trampoline_script)

if (NOT trampoline_script MATCHES [[file\(GLOB _cmake_cxx_module_includes "\${CMAKE_CURRENT_LIST_DIR}/cxx-modules-\*\.cmake"\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find C++ module property per-config script inclusion(s)")
endif ()

set(any_exists 0)
foreach (config IN ITEMS noconfig Debug Release RelWithDebInfo MinSizeRel)
  if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Export/eee57a7e91412f1be699e9b63fa9d601/cxx-modules/cxx-modules-${config}.cmake")
    continue ()
  endif ()
  set(any_exists 1)

  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Export/eee57a7e91412f1be699e9b63fa9d601/cxx-modules/cxx-modules-${config}.cmake" config_script)

  if (NOT config_script MATCHES "include\\(\"\\\${CMAKE_CURRENT_LIST_DIR}/target-export-name-${config}\\.cmake\"\\)")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find C++ module per-target property script inclusion")
  endif ()
endforeach ()

if (NOT any_exists)
  list(APPEND RunCMake_TEST_FAILED
    "No per-configuration target files exist.")
endif ()

string(REPLACE ";" "; " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
