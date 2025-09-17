# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard()

macro(__emscripten_clang lang)
  set(CMAKE_SHARED_LIBRARY_SONAME_${lang}_FLAG "-Wl,-soname,")

  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_INCLUDES 1)
  set(CMAKE_${lang}_COMPILE_OBJECT
    "<CMAKE_${lang}_COMPILER> -c <SOURCE> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -fPIC")

  get_property(_TARGET_SUPPORTS_SHARED_LIBS GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
  if(_TARGET_SUPPORTS_SHARED_LIBS)
    # Emscripten requires '-sMAIN_MODULE' and '-sSIDE_MODULE' to distinguish
    # linking executables from linking shared libraries.
    set(CMAKE_${lang}_LINK_EXECUTABLE
      "<CMAKE_${lang}_COMPILER> <FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -sMAIN_MODULE")
    set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-sSIDE_MODULE")
  else()
    # Emscripten provides a combined toolchain file and platform module that
    # predates CMake upstream support.  It turns off support for shared libraries.
    # Avoid linking with '-sMAIN_MODULE' or '-sSIDE_MODULE' in that case.
    set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "")
  endif()
endmacro()
