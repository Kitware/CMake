file(READ "${RunCMake_TEST_BINARY_DIR}/lib/cmake/export-modules/export-modules-targets.cmake" export_script)

if (NOT export_script MATCHES [[include\("\${CMAKE_CURRENT_LIST_DIR}/cxx-modules/cxx-modules-7eb8e1b4d8a1\.cmake"\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find C++ module property script inclusion")
endif ()

file(READ "${RunCMake_TEST_BINARY_DIR}/lib/cmake/export-modules/cxx-modules/cxx-modules-7eb8e1b4d8a1.cmake" trampoline_script)

if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  if (NOT trampoline_script MATCHES [[include\("\${CMAKE_CURRENT_LIST_DIR}/cxx-modules-7eb8e1b4d8a1-[^.]*\.cmake" OPTIONAL\)]])
    list(APPEND RunCMake_TEST_FAILED
      "Could not find C++ module property per-config script inclusion(s)")
  endif ()
else ()
  if (NOT trampoline_script MATCHES [[include\("\${CMAKE_CURRENT_LIST_DIR}/cxx-modules-7eb8e1b4d8a1-[^.]*\.cmake"\)]])
    list(APPEND RunCMake_TEST_FAILED
      "Could not find C++ module property per-config script inclusion(s)")
  endif ()
endif ()

set(any_exists 0)
foreach (config IN ITEMS noconfig Debug Release RelWithDebInfo MinSizeRel)
  if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/lib/cmake/export-modules/cxx-modules/cxx-modules-7eb8e1b4d8a1-${config}.cmake")
    continue ()
  endif ()
  set(any_exists 1)

  file(READ "${RunCMake_TEST_BINARY_DIR}/lib/cmake/export-modules/cxx-modules/cxx-modules-7eb8e1b4d8a1-${config}.cmake" config_script)

  if (NOT config_script MATCHES "include\\(\"\\\${CMAKE_CURRENT_LIST_DIR}/target-export-name-${config}\\.cmake\"\\)")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find C++ module per-target property script inclusion")
  endif ()
endforeach ()

if (NOT any_exists)
  list(APPEND RunCMake_TEST_FAILED
    "No per-configuration target files exist.")
endif ()
