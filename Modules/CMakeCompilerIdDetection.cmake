
#=============================================================================
# Copyright 2014 Stephen Kelly <steveire@gmail.com>
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

function(_readFile file)
  include(${file})
  get_filename_component(name ${file} NAME_WE)
  string(REGEX REPLACE "-.*" "" CompilerId ${name})
  set(_compiler_id_version_compute_${CompilerId} ${_compiler_id_version_compute} PARENT_SCOPE)
  set(_compiler_id_pp_test_${CompilerId} ${_compiler_id_pp_test} PARENT_SCOPE)
endfunction()

include(${CMAKE_CURRENT_LIST_DIR}/CMakeParseArguments.cmake)

function(compiler_id_detection outvar lang)

  file(GLOB lang_files
    "${CMAKE_ROOT}/Modules/Compiler/*-${lang}-DetermineCompiler.cmake")

  if (NOT lang STREQUAL Fortran)
    file(GLOB non_lang_files
      "${CMAKE_ROOT}/Modules/Compiler/*-DetermineCompiler.cmake")
  endif()

  set(files ${lang_files} ${non_lang_files})
  if (files)
    foreach(file ${files})
      _readFile(${file})
    endforeach()

    set(options ID_STRING VERSION_STRINGS)
    cmake_parse_arguments(CID "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})
    if (CID_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Unrecognized arguments: \"${CID_UNPARSED_ARGUMENTS}\"")
    endif()

    set(ordered_compilers
      # Order is relevant here. For example, compilers which pretend to be
      # GCC must appear before the actual GCC.
      Comeau
      Intel
      PathScale
      AppleClang
      Clang
      Embarcadero
      Borland
      Watcom
      OpenWatcom
      SunPro
      HP
      Compaq
      zOS
      XL
      VisualAge
      PGI
      Cray
      TI
      SCO
      GNU
      MSVC
      ADSP
      IAR
      MIPSpro)

    set(pp_if "#if")
    if (CID_VERSION_STRINGS)
      set(CMAKE_${lang}_COMPILER_ID_CONTENT "/* Version number components: V=Version, R=Revision, P=Patch
   Version date components:   YYYY=Year, MM=Month,   DD=Day  */\n")
    endif()

    foreach(Id ${ordered_compilers})
      if (NOT _compiler_id_pp_test_${Id})
        message(FATAL_ERROR "No preprocessor test for \"${Id}\"")
      endif()
      set(id_content "${pp_if} ${_compiler_id_pp_test_${Id}}\n")
      if (CID_ID_STRING)
        set(id_content "${id_content}# define COMPILER_ID \"${Id}\"")
      endif()
      if (CID_VERSION_STRINGS)
        set(id_content "${id_content}${_compiler_id_version_compute_${Id}}\n")
      endif()
      set(CMAKE_${lang}_COMPILER_ID_CONTENT "${CMAKE_${lang}_COMPILER_ID_CONTENT}\n${id_content}")
      set(pp_if "#elif")
    endforeach()

    set(platform_compiler_detection "
/* These compilers are either not known or too old to define an
  identification macro.  Try to identify the platform and guess that
  it is the native compiler.  */
#elif defined(__sgi)
# define COMPILER_ID \"MIPSpro\"

#elif defined(__hpux) || defined(__hpua)
# define COMPILER_ID \"HP\"

#else /* unknown compiler */
# define COMPILER_ID \"\"

#endif")

    set(CMAKE_${lang}_COMPILER_ID_CONTENT "${CMAKE_${lang}_COMPILER_ID_CONTENT}\n${platform_compiler_detection}")
  endif()

  set(${outvar} ${CMAKE_${lang}_COMPILER_ID_CONTENT} PARENT_SCOPE)
endfunction()
