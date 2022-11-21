set(CMake_TEST_CXXModules_UUID "a246741c-d067-4019-a8fb-3d16b0c9d1d3")

string(CONCAT CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE
  "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -E -x c++ <SOURCE>"
  " -MT <DYNDEP_FILE> -MD -MF <DEP_FILE>"
  " -fmodules-ts -fdeps-file=<DYNDEP_FILE> -fdeps-target=<OBJECT> -fdeps-format=p1689r5"
  " -o <PREPROCESSED_SOURCE>")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT "gcc")
string(CONCAT CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG
  # Turn on modules.
  "-fmodules-ts"
  # Read the module mapper file.
  " -fmodule-mapper=<MODULE_MAP_FILE>"
  # Make sure dependency tracking is enabled (missing from `try_*`).
  " -MD"
  # Suppress `CXX_MODULES +=` from generated depfile snippets.
  " -fdeps-format=p1689r5"
  # Force C++ as a language.
  " -x c++")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_BMI_ONLY_FLAG "-fmodule-only")
