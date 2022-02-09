# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_INTEL)
  return()
endif()
set(__WINDOWS_INTEL 1)

include(Platform/Windows-MSVC)
macro(__windows_compiler_intel lang)
  __windows_compiler_msvc(${lang})

  # For DPCPP other offload cases, some link flags need to go to the compiler
  # driver and others need to go to the linker.  Pass the compiler linking flags
  # in CMAKE_${lang}_LINK_FLAGS and linker flags in LINK_FLAGS
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "${_CMAKE_VS_LINK_EXE}<CMAKE_${lang}_COMPILER> ${CMAKE_CL_NOLOGO} <CMAKE_${lang}_LINK_FLAGS> <OBJECTS> ${CMAKE_START_TEMP_FILE} /link /out:<TARGET> /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PLATFORM_LINK_FLAGS} <LINK_FLAGS> <LINK_LIBRARIES>${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "${_CMAKE_VS_LINK_DLL}<CMAKE_${lang}_COMPILER> ${CMAKE_CL_NOLOGO} <CMAKE_${lang}_LINK_FLAGS> <OBJECTS> ${CMAKE_START_TEMP_FILE} -LD -link /out:<TARGET> /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PLATFORM_LINK_FLAGS} <LINK_FLAGS> <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
  if (NOT "${lang}" STREQUAL "Fortran" OR CMAKE_${lang}_COMPILER_VERSION VERSION_GREATER_EQUAL 2022.1)
    # The Fortran driver does not support -fuse-ld=llvm-lib before compiler version 2022.1
    set(CMAKE_${lang}_CREATE_STATIC_LIBRARY
      "<CMAKE_${lang}_COMPILER> ${CMAKE_CL_NOLOGO} <CMAKE_${lang}_LINK_FLAGS> <OBJECTS> ${CMAKE_START_TEMP_FILE} -fuse-ld=llvm-lib -o <TARGET> -link <LINK_FLAGS> <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
  endif()

  set(CMAKE_DEPFILE_FLAGS_${lang} "-QMD -QMT <DEP_TARGET> -QMF <DEP_FILE>")
  set(CMAKE_${lang}_DEPFILE_FORMAT gcc)
endmacro()
