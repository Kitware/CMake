# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.
include_guard()

set(CMAKE_LIBRARY_PATH_FLAG "-libpath:")
set(CMAKE_LINK_DEF_FILE_FLAG "-def:")
set(CMAKE_LINK_LIBRARY_FLAG "")

macro(__windows_compiler_pellesc lang)
  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY "<CMAKE_AR> <LINK_FLAGS> -out:<TARGET> <OBJECTS>")

  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "<CMAKE_LINKER> <OBJECTS> -out:<TARGET> -implib:<TARGET_IMPLIB> -dll -version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR> <LINK_FLAGS> <LINK_LIBRARIES>")
  set(CMAKE_${lang}_CREATE_SHARED_MODULE "${CMAKE_${lang}_CREATE_SHARED_LIBRARY}")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "")

  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_LINKER> <OBJECTS> -out:<TARGET> -implib:<TARGET_IMPLIB> -version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR> <LINK_FLAGS> <LINK_LIBRARIES>")

  set(CMAKE_${lang}_LINK_DEF_FILE_FLAG "${CMAKE_LINK_DEF_FILE_FLAG}")

  set(CMAKE_${lang}_CREATE_WIN32_EXE "-subsystem:windows")
  set(CMAKE_${lang}_CREATE_CONSOLE_EXE "-subsystem:console")

  if(NOT CMAKE_RC_COMPILER_INIT)
    set(CMAKE_RC_COMPILER_INIT porc)
  endif()
  enable_language(RC)
endmacro()
