# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.
include_guard()

set(CMAKE_LIBRARY_PATH_FLAG "-libpath:")
set(CMAKE_LINK_DEF_FILE_FLAG "-def:")
set(CMAKE_LINK_LIBRARY_FLAG "")

set(CMAKE_BUILD_TYPE_INIT Debug)

# Select a default architecture based on the LIB search path.
string(TOLOWER "$ENV{LIB}" _PellesC_LIB)
if(_PellesC_LIB MATCHES [[[\/]+lib[\/]+win64[\/]*(;|$)]])
  set(_PellesC_ARCH "x64")
elseif(_PellesC_LIB MATCHES [[[\/]+lib[\/]+win[\/]*(;|$)]])
  set(_PellesC_ARCH "x86")
else()
  set(_PellesC_ARCH "")
endif()

macro(__windows_compiler_pellesc lang)
  if(_PellesC_ARCH)
    set(_PellesC_LINK_FLAGS " -machine:${_PellesC_ARCH}")
  else()
    set(_PellesC_LINK_FLAGS "")
  endif()

  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY "<CMAKE_AR> <LINK_FLAGS> -out:<TARGET> <OBJECTS>")

  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "<CMAKE_LINKER> <OBJECTS> ${CMAKE_START_TEMP_FILE} -out:<TARGET> -implib:<TARGET_IMPLIB> -dll -version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PellesC_LINK_FLAGS} <LINK_FLAGS> <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_SHARED_MODULE "${CMAKE_${lang}_CREATE_SHARED_LIBRARY}")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "")

  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS 1)
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_LINKER> <OBJECTS> ${CMAKE_START_TEMP_FILE} -out:<TARGET> -implib:<TARGET_IMPLIB> -version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PellesC_LINK_FLAGS} <LINK_FLAGS> <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")

  set(CMAKE_${lang}_LINK_DEF_FILE_FLAG "${CMAKE_LINK_DEF_FILE_FLAG}")

  set(CMAKE_${lang}_CREATE_WIN32_EXE "-subsystem:windows")
  set(CMAKE_${lang}_CREATE_CONSOLE_EXE "-subsystem:console")

  if(NOT CMAKE_RC_COMPILER_INIT)
    set(CMAKE_RC_COMPILER_INIT porc)
  endif()
  enable_language(RC)

  unset(_PellesC_LINK_FLAGS)
endmacro()

foreach (t IN ITEMS EXE SHARED MODULE)
  string(APPEND CMAKE_${t}_LINKER_FLAGS_DEBUG_INIT " -debug -debugtype:both -dbg")
  string(APPEND CMAKE_${t}_LINKER_FLAGS_RELWITHDEBINFO_INIT " -debug -debugtype:both -dbg")
endforeach ()
