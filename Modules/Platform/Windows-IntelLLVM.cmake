# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_INTEL_LLVM)
  return()
endif()
set(__WINDOWS_INTEL_LLVM 1)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  # MSBuild invokes the "link" tool directly.
  set(_IntelLLVM_LINKER_WRAPPER_FLAG "")
  set(_IntelLLVM_LINKER_WRAPPER_FLAG_SEP "")
else()
  # Our rules below drive linking through the compiler front-end.
  # Wrap flags meant for the linker.
  set(_IntelLLVM_LINKER_WRAPPER_FLAG "/Qoption,link,")
  set(_IntelLLVM_LINKER_WRAPPER_FLAG_SEP ",")
endif()
set(_Wl "${_IntelLLVM_LINKER_WRAPPER_FLAG}")
include(Platform/Windows-MSVC)
unset(_Wl)

macro(__windows_compiler_intel lang)
  __windows_compiler_msvc(${lang})

  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG "${_IntelLLVM_LINKER_WRAPPER_FLAG}")
  set(CMAKE_${lang}_LINKER_WRAPPER_FLAG_SEP "${_IntelLLVM_LINKER_WRAPPER_FLAG_SEP}")
  set(CMAKE_${lang}_CREATE_WIN32_EXE "${CMAKE_${lang}_LINKER_WRAPPER_FLAG}/subsystem:windows")
  set(CMAKE_${lang}_CREATE_CONSOLE_EXE "${CMAKE_${lang}_LINKER_WRAPPER_FLAG}/subsystem:console")
  set(CMAKE_LINK_DEF_FILE_FLAG "${CMAKE_${lang}_LINKER_WRAPPER_FLAG}/DEF:")
  set(CMAKE_LIBRARY_PATH_FLAG "${CMAKE_${lang}_LINKER_WRAPPER_FLAG}/LIBPATH:")

  # Features for LINK_LIBRARY generator expression
  if(MSVC_VERSION GREATER "1900")
    ## WHOLE_ARCHIVE: Force loading all members of an archive
    set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "LINKER:/WHOLEARCHIVE:<LIBRARY>")
    set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)
  endif()

  set(CMAKE_${lang}_LINK_EXECUTABLE
    "${_CMAKE_VS_LINK_EXE}<CMAKE_${lang}_COMPILER> ${CMAKE_CL_NOLOGO} <CMAKE_${lang}_LINK_FLAGS> <OBJECTS> ${CMAKE_START_TEMP_FILE} <LINK_FLAGS> <LINK_LIBRARIES> /link /out:<TARGET> /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PLATFORM_LINK_FLAGS} ${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "${_CMAKE_VS_LINK_DLL}<CMAKE_${lang}_COMPILER> ${CMAKE_CL_NOLOGO} <CMAKE_${lang}_LINK_FLAGS> <OBJECTS> ${CMAKE_START_TEMP_FILE} -LD <LINK_FLAGS> <LINK_LIBRARIES> -link /out:<TARGET> /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PLATFORM_LINK_FLAGS} ${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_SHARED_MODULE ${CMAKE_${lang}_CREATE_SHARED_LIBRARY})
  if (NOT "${lang}" STREQUAL "Fortran" OR CMAKE_${lang}_COMPILER_VERSION VERSION_GREATER_EQUAL 2022.1)
    # The Fortran driver does not support -fuse-ld=llvm-lib before compiler version 2022.1
    set(CMAKE_${lang}_CREATE_STATIC_LIBRARY
      "<CMAKE_${lang}_COMPILER> ${CMAKE_CL_NOLOGO} <CMAKE_${lang}_LINK_FLAGS> <OBJECTS> ${CMAKE_START_TEMP_FILE} -fuse-ld=llvm-lib -o <TARGET> <LINK_FLAGS> <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
  endif()

  set(CMAKE_DEPFILE_FLAGS_${lang} "-QMD -QMT <DEP_TARGET> -QMF <DEP_FILE>")
  set(CMAKE_${lang}_DEPFILE_FORMAT gcc)
endmacro()
