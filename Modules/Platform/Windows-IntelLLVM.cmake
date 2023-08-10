# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_INTEL_LLVM)
  return()
endif()
set(__WINDOWS_INTEL_LLVM 1)

# Platform/Windows-MSVC adds some linking options icx/ifx do not understand,
# but that need to be passed to the linker.  Wrap all the linking options from
# Platform/Windows-MSVC so that the compiler will hand them off to the linker
# without interpreting them.

# Save original CMAKE_${t}_LINKER_FLAGS_INIT
foreach(t EXE SHARED MODULE STATIC)
  set(_saved_cmake_${t}_linker_flags_init ${CMAKE_${t}_LINKER_FLAGS_INIT})
  set(CMAKE_${t}_LINKER_FLAGS_INIT "")
endforeach()
include(Platform/Windows-MSVC)
# Wrap linker flags from Windows-MSVC
set(_IntelLLVM_LINKER_WRAPPER_FLAG "/Qoption,link,")
set(_IntelLLVM_LINKER_WRAPPER_FLAG_SEP ",")
foreach(t EXE SHARED MODULE STATIC)
  set(_wrapped_linker_flags "")
  foreach(flag ${CMAKE_${t}_LINKER_FLAGS_INIT})
    string(STRIP ${flag} flag)
    list(APPEND _wrapped_linker_flags "${_IntelLLVM_LINKER_WRAPPER_FLAG}${flag}")
  endforeach()
  set(CMAKE_${t}_LINKER_FLAGS_INIT "")
  list(APPEND CMAKE_${t}_LINKER_FLAGS_INIT
    ${_saved_cmake_${t}_linker_flags_init} ${_wrapped_linker_flags})
endforeach()

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
