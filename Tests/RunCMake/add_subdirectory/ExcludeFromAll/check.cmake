# Use globbing to check if exes / libs were built because determining
# exactly where these files will live inside a CMake -P script is
# pretty challenging.

file(READ "${RunCMake_TEST_BINARY_DIR}/main.txt" main_exe)
file(READ "${RunCMake_TEST_BINARY_DIR}/bar.txt" bar_lib)
file(READ "${RunCMake_TEST_BINARY_DIR}/baz.txt" baz_lib)

set(found_main FALSE)
file(GLOB_RECURSE files
  LIST_DIRECTORIES FALSE
  RELATIVE "${RunCMake_TEST_BINARY_DIR}"
  "${RunCMake_TEST_BINARY_DIR}/*")
foreach (file IN LISTS files)
  if (file MATCHES "${main_exe}")
    set(found_main TRUE)
  endif()
endforeach()
if (NOT found_main)
  set(RunCMake_TEST_FAILED "'main' missing from ${RunCMake_TEST_BINARY_DIR}")
endif()

set(found_bar FALSE)
set(found_baz FALSE)
file(GLOB_RECURSE files
  LIST_DIRECTORIES FALSE
  RELATIVE "${RunCMake_TEST_BINARY_DIR}/ExcludeFromAll"
  "${RunCMake_TEST_BINARY_DIR}/ExcludeFromAll/*")
foreach (file IN LISTS files)
  if (file MATCHES "${bar_lib}")
    set(found_bar TRUE)
  endif()
  if (file MATCHES "${baz_lib}")
    set(found_baz TRUE)
  endif()
endforeach()
if (found_bar)
  set(RunCMake_TEST_FAILED
    "'bar' was not excluded from ${RunCMake_TEST_BINARY_DIR}/ExcludeFromAll")
endif()
if (NOT found_baz)
  set(RunCMake_TEST_FAILED
    "'baz' missing from ${RunCMake_TEST_BINARY_DIR}/ExcludeFromAll")
endif()
