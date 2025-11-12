enable_language(CXX)

if(CMAKE_CXX_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

######################################################################

file(WRITE ${CMAKE_BINARY_DIR}/pch.cxx [=[
void nothing()
{
}
]=])

file(WRITE ${CMAKE_BINARY_DIR}/string.hxx [=[
#include <string.h>

namespace std {
  struct string
  {
    char storage[20];

    string(const char* s) {
      strcpy(storage, s);
    }

    const char* c_str() const {
      return storage;
    }
  };
}
]=])

add_library(pch-generator ${CMAKE_BINARY_DIR}/pch.cxx)
target_precompile_headers(pch-generator PRIVATE ${CMAKE_BINARY_DIR}/string.hxx)

######################################################################

file(WRITE ${CMAKE_BINARY_DIR}/message.cxx [=[
#include "message.hxx"

#ifndef HAVE_PCH_SUPPORT
  #include "string.hxx"
#endif

const char* message()
{
  static std::string greeting("hi there");
  return greeting.c_str();
}
]=])

file(WRITE ${CMAKE_BINARY_DIR}/message.hxx [=[
const char* message();
]=])

add_library(ignored_props ${CMAKE_BINARY_DIR}/message.cxx)
target_precompile_headers(ignored_props REUSE_FROM pch-generator)
target_include_directories(ignored_props PRIVATE ${CMAKE_BINARY_DIR})

set_target_properties(ignored_props
  PROPERTIES
    COMPILE_PDB_NAME "NOT_USED"
    COMPILE_PDB_NAME_DEBUG "NOT_USED_DEBUG"
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/mycustomdir"
    COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/mycustomdir_debug")
