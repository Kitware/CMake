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

add_library(reuse_append ${CMAKE_BINARY_DIR}/message.cxx)
target_include_directories(reuse_append PRIVATE ${CMAKE_BINARY_DIR})
set_property(TARGET reuse_append PROPERTY PRECOMPILE_HEADERS_REUSE_FROM "pch-")
set_property(TARGET reuse_append APPEND_STRING PROPERTY PRECOMPILE_HEADERS_REUSE_FROM "generator")
