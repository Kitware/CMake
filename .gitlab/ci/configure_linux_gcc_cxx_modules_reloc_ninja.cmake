# "Misplace" the `libstdc++.modules.json` file so that
# `CMAKE_CXX_STDLIB_MODULES_JSON` is needed to use `import std`.
set(gcc_prefix "/opt/gcc-importstd")
set(CMake_TEST_CXX_STDLIB_MODULES_JSON
  "${gcc_prefix}/lib64.reloc/libstdc++.modules.json"
  CACHE FILEPATH "")
file(MAKE_DIRECTORY
  "${gcc_prefix}/lib64.reloc")
file(RENAME
  "${gcc_prefix}/lib64/libstdc++.modules.json"
  "${CMake_TEST_CXX_STDLIB_MODULES_JSON}")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
