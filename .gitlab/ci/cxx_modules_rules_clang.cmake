set(CMake_TEST_CXXModules_UUID "a246741c-d067-4019-a8fb-3d16b0c9d1d3")

set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
string(CONCAT CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE
  "${CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS}"
  " -format=p1689 --p1689-targeted-file-name=<SOURCE> --p1689-targeted-output=<OBJECT> "
  " --p1689-makeformat-output=<DEP_FILE>"
  " --"
  " <DEFINES> <INCLUDES> <FLAGS> -x c++ <SOURCE>"
  " -MT <DYNDEP_FILE> -MD"
  " > <DYNDEP_FILE>")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT "clang")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG "@<MODULE_MAP_FILE>")

# Default to C++ extensions being off. Clang's modules support have trouble
# with extensions right now.
set(CMAKE_CXX_EXTENSIONS OFF)
