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

add_library(pch_before_reuse_pch ${CMAKE_BINARY_DIR}/message.cxx)
target_precompile_headers(pch_before_reuse_pch PRIVATE "${CMAKE_BINARY_DIR}/string.hxx")
target_precompile_headers(pch_before_reuse_pch REUSE_FROM pch-generator)
set_property(TARGET pch_before_reuse_pch PROPERTY PRECOMPILE_HEADERS_REUSE_FROM)
target_include_directories(pch_before_reuse_pch PRIVATE ${CMAKE_BINARY_DIR})

add_library(pch_before_reuse_reuse ${CMAKE_BINARY_DIR}/message.cxx)
target_precompile_headers(pch_before_reuse_reuse PRIVATE "${CMAKE_BINARY_DIR}/string.hxx")
target_precompile_headers(pch_before_reuse_reuse REUSE_FROM pch-generator)
set_property(TARGET pch_before_reuse_reuse PROPERTY PRECOMPILE_HEADERS "")
target_include_directories(pch_before_reuse_reuse PRIVATE ${CMAKE_BINARY_DIR})

add_library(reuse_before_pch_pch ${CMAKE_BINARY_DIR}/message.cxx)
target_precompile_headers(reuse_before_pch_pch REUSE_FROM pch-generator)
target_precompile_headers(reuse_before_pch_pch PRIVATE "${CMAKE_BINARY_DIR}/string.hxx")
set_property(TARGET reuse_before_pch_pch PROPERTY PRECOMPILE_HEADERS_REUSE_FROM)
target_include_directories(reuse_before_pch_pch PRIVATE ${CMAKE_BINARY_DIR})

add_library(reuse_before_pch_reuse ${CMAKE_BINARY_DIR}/message.cxx)
target_precompile_headers(reuse_before_pch_reuse REUSE_FROM pch-generator)
target_precompile_headers(reuse_before_pch_reuse PRIVATE "${CMAKE_BINARY_DIR}/string.hxx")
set_property(TARGET reuse_before_pch_reuse PROPERTY PRECOMPILE_HEADERS "")
target_include_directories(reuse_before_pch_reuse PRIVATE ${CMAKE_BINARY_DIR})
