# Emscripten provides a combined toolchain file and platform module
# that predates CMake upstream support.  Avoid interfering with it.
if(CMAKE_SYSTEM_VERSION EQUAL 1)
  if(CMAKE_TOOLCHAIN_FILE MATCHES [[/Modules/Platform/Emscripten\.cmake$]])
    include("${CMAKE_TOOLCHAIN_FILE}")
    return()
  endif()
endif()

set(CMAKE_SHARED_LIBRARY_LINK_C_WITH_RUNTIME_PATH ON)

set(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")
set(CMAKE_EXECUTABLE_SUFFIX ".js")

set(CMAKE_DL_LIBS "")
