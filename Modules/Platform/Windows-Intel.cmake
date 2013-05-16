
#=============================================================================
# Copyright 2002-2010 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_INTEL)
  return()
endif()
set(__WINDOWS_INTEL 1)

# make sure to enable languages after setting configuration types
enable_language(RC)
set(CMAKE_COMPILE_RESOURCE "rc <FLAGS> /fo<OBJECT> <SOURCE>")

set(CMAKE_LIBRARY_PATH_FLAG "-LIBPATH:")
set(CMAKE_LINK_LIBRARY_FLAG "")
set(WIN32 1)
if(CMAKE_VERBOSE_MAKEFILE)
  set(CMAKE_CL_NOLOGO)
else()
  set(CMAKE_CL_NOLOGO "/nologo")
endif()
set(CMAKE_COMPILE_RESOURCE "rc <FLAGS> /fo<OBJECT> <SOURCE>")
set(CMAKE_CREATE_WIN32_EXE /subsystem:windows)
set(CMAKE_CREATE_CONSOLE_EXE /subsystem:console)

# default to Debug builds
#set(CMAKE_BUILD_TYPE_INIT Debug)
set(CMAKE_BUILD_TYPE_INIT Release)

set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT}")

# executable linker flags
set (CMAKE_LINK_DEF_FILE_FLAG "/DEF:")
if(MSVC_C_ARCHITECTURE_ID)
  set(_MACHINE_ARCH_FLAG "/machine:${MSVC_C_ARCHITECTURE_ID}")
elseif(MSVC_CXX_ARCHITECTURE_ID)
  set(_MACHINE_ARCH_FLAG "/machine:${MSVC_CXX_ARCHITECTURE_ID}")
elseif(MSVC_Fortran_ARCHITECTURE_ID)
  set(_MACHINE_ARCH_FLAG "/machine:${MSVC_Fortran_ARCHITECTURE_ID}")
endif()
set (CMAKE_EXE_LINKER_FLAGS_INIT "/INCREMENTAL:YES ${_MACHINE_ARCH_FLAG}")
set (CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "/debug")
set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "/debug")

set (CMAKE_SHARED_LINKER_FLAGS_INIT ${CMAKE_EXE_LINKER_FLAGS_INIT})
set (CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT})
set (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT})
set (CMAKE_MODULE_LINKER_FLAGS_INIT ${CMAKE_SHARED_LINKER_FLAGS_INIT})
set (CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT ${CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT})
set (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT})

include("${CMAKE_PLATFORM_INFO_DIR}/CMakeIntelInformation.cmake" OPTIONAL)

if(NOT _INTEL_XILINK_TEST_RUN)
  execute_process(COMMAND xilink /?
    ERROR_VARIABLE _XILINK_ERR
    OUTPUT_VARIABLE _XILINK_HELP)
  if(_XILINK_HELP MATCHES MANIFEST)
    set(_INTEL_COMPILER_SUPPORTS_MANIFEST 1)
  endif()
  if(NOT  EXISTS "${CMAKE_PLATFORM_INFO_DIR}/CMakeIntelInformation.cmake")
    file(WRITE ${CMAKE_PLATFORM_INFO_DIR}/CMakeIntelInformation.cmake
      "
set(_INTEL_XILINK_TEST_RUN 1)
set(_INTEL_COMPILER_SUPPORTS_MANIFEST ${_INTEL_COMPILER_SUPPORTS_MANIFEST})
")
  endif()
endif()

macro(__windows_compiler_intel lang)
  set(CMAKE_${lang}_COMPILE_OBJECT
    "<CMAKE_${lang}_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_CL_NOLOGO}${_COMPILE_${lang}} /Fo<OBJECT> /Fd<OBJECT_DIR>/ <DEFINES> <FLAGS> -c <SOURCE>${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_PREPROCESSED_SOURCE
    "<CMAKE_${lang}_COMPILER> > <PREPROCESSED_SOURCE> ${CMAKE_START_TEMP_FILE} ${CMAKE_CL_NOLOGO}${_COMPILE_${lang}} <DEFINES> <FLAGS> -E <SOURCE>${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS 1)
  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY  "lib ${CMAKE_CL_NOLOGO} <LINK_FLAGS> /out:<TARGET> <OBJECTS> ")
  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "xilink ${CMAKE_CL_NOLOGO} ${CMAKE_START_TEMP_FILE}  /out:<TARGET> /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> /dll  <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_SHARED_MODULE "${CMAKE_${lang}_CREATE_SHARED_LIBRARY}")
  set(CMAKE_${lang}_COMPILER_LINKER_OPTION_FLAG_EXECUTABLE "/link")
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "<CMAKE_${lang}_COMPILER> ${CMAKE_CL_NOLOGO} ${CMAKE_START_TEMP_FILE} <FLAGS> /Fe<TARGET> <OBJECTS> /link /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_FLAGS_INIT "/DWIN32 /D_WINDOWS /W3 /Zm1000${_FLAGS_${lang}}")
  set(CMAKE_${lang}_FLAGS_DEBUG_INIT "/D_DEBUG /MDd /Zi /Od /RTC1")
  set(CMAKE_${lang}_FLAGS_MINSIZEREL_INIT "/DNDEBUG /MD /O1")
  set(CMAKE_${lang}_FLAGS_RELEASE_INIT "/DNDEBUG /MD /O2")
  set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT "/DNDEBUG /MD /Zi /O2")

  if(_INTEL_COMPILER_SUPPORTS_MANIFEST)
    set(CMAKE_${lang}_LINK_EXECUTABLE
      "<CMAKE_COMMAND> -E vs_link_exe ${CMAKE_${lang}_LINK_EXECUTABLE}")
    set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
      "<CMAKE_COMMAND> -E vs_link_dll ${CMAKE_${lang}_CREATE_SHARED_LIBRARY}")
    set(CMAKE_${lang}_CREATE_SHARED_MODULE
      "<CMAKE_COMMAND> -E vs_link_dll ${CMAKE_${lang}_CREATE_SHARED_MODULE}")
  endif()
endmacro()
