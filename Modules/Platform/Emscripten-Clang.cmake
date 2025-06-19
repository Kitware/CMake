# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard()

macro(__emscripten_clang lang)
  set(CMAKE_SHARED_LIBRARY_SONAME_${lang}_FLAG "-Wl,-soname,")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-sSIDE_MODULE")

  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_INCLUDES 1)
  set(CMAKE_${lang}_COMPILE_OBJECT
    "<CMAKE_${lang}_COMPILER> -c <SOURCE> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -fPIC")
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_${lang}_COMPILER> <FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -sMAIN_MODULE")
endmacro()
