enable_language(CXX)

if(CMAKE_CXX_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

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

add_library(reuse_decl_order ${CMAKE_BINARY_DIR}/message.cxx)
target_precompile_headers(reuse_decl_order REUSE_FROM pch-generator)
target_include_directories(reuse_decl_order PRIVATE ${CMAKE_BINARY_DIR})

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
