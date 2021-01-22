cmake_minimum_required(VERSION 3.18)

project(PchReuseFromObjLib)

set(CMAKE_PCH_WARN_INVALID OFF)

if(CMAKE_CXX_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

######################################################################

file(WRITE ${CMAKE_BINARY_DIR}/CONFIG/config.hxx "/*empty*/\n")

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

add_library(pch-generator OBJECT ${CMAKE_BINARY_DIR}/pch.cxx)
set_property(TARGET pch-generator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_precompile_headers(pch-generator PRIVATE ${CMAKE_BINARY_DIR}/string.hxx)

target_include_directories(pch-generator PRIVATE ${CMAKE_BINARY_DIR}/CONFIG)

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
#include "config.hxx"
#ifdef WIN32_BUILD_SHARED
  #ifdef BUILD_LIBRARY
    #define MESSAGE_EXPORT __declspec(dllexport)
  #else
    #define MESSAGE_EXPORT __declspec(dllimport)
  #endif
#else
  #define MESSAGE_EXPORT
#endif

MESSAGE_EXPORT const char* message();
]=])

######################################################################

file(WRITE ${CMAKE_BINARY_DIR}/main.cxx [=[
#include "message.hxx"
#include <string.h>

int main()
{
  return strcmp(message(), "hi there");
}
]=])

######################################################################

enable_testing()

function(add_library_and_executable type)
  add_library(message_${type} ${type} ${CMAKE_BINARY_DIR}/message.cxx)
  target_precompile_headers(message_${type} REUSE_FROM pch-generator)

  set_property(TARGET message_${type} PROPERTY POSITION_INDEPENDENT_CODE ON)
  set_property(TARGET message_${type} PROPERTY DEFINE_SYMBOL "")

  if (WIN32 AND type STREQUAL "SHARED")
    file(WRITE ${CMAKE_BINARY_DIR}/SHARED/config.hxx [=[
      #define BUILD_LIBRARY
      #define WIN32_BUILD_SHARED
    ]=])
    target_include_directories(message_${type} PRIVATE ${CMAKE_BINARY_DIR}/SHARED)

    # Workaround for VS2008, the compiler fails with
    # c1xx : fatal error C1083: Cannot open source file: '_WINDLL': No such file or directory
    file(WRITE ${CMAKE_BINARY_DIR}/_WINDLL "/*empty*/\n")
  else()
    target_include_directories(message_${type} PRIVATE ${CMAKE_BINARY_DIR}/CONFIG)
  endif()

  add_executable(main_${type} ${CMAKE_BINARY_DIR}/main.cxx)
  target_include_directories(main_${type} PRIVATE ${CMAKE_BINARY_DIR})

  if (WIN32 AND type STREQUAL "SHARED")
    file(WRITE ${CMAKE_BINARY_DIR}/main_SHARED/config.hxx "#define WIN32_BUILD_SHARED\n")
    target_include_directories(main_${type} PRIVATE ${CMAKE_BINARY_DIR}/main_SHARED)
  else()
    target_include_directories(main_${type} PRIVATE ${CMAKE_BINARY_DIR}/CONFIG)
  endif()

  target_link_libraries(main_${type} PRIVATE message_${type})

  add_test(NAME main_${type} COMMAND main_${type})
endfunction()

foreach(type OBJECT STATIC SHARED)
  add_library_and_executable(${type})
endforeach()
