
#=============================================================================
# Copyright 2013 Stephen Kelly <steveire@gmail.com>
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

# Should we enable the highest supported lang flag?

include(${CMAKE_CURRENT_LIST_DIR}/CMakeParseArguments.cmake)

function(_load_compiler_variables CompilerId lang)
  include("${CMAKE_ROOT}/Modules/Compiler/${CompilerId}-${lang}-FeatureTests.cmake" OPTIONAL)
  if (NOT _cmake_compiler_test_macro)
    message(FATAL_ERROR "Compiler ${CompilerId} does not define _cmake_compiler_test_macro")
  endif()
  foreach(feature ${ARGN})
    set(x ${_cmake_feature_tests_${feature}})
    if (_cmake_feature_test_${feature})
      set(n "defined (${_cmake_compiler_test_macro})")
      if (NOT _cmake_feature_test_${feature} STREQUAL "1")
        set(n "(${n} && ${_cmake_feature_test_${feature}})")
      endif()
      list(APPEND x ${n})
      set(_cmake_feature_tests_${feature} ${x} PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

function(write_compiler_detection_header
    file_keyword file_arg
    prefix_keyword prefix_arg
    )
  if (NOT file_keyword STREQUAL FILE)
    message(FATAL_ERROR "Wrong parameters for function.")
  endif()
  if (NOT prefix_keyword STREQUAL PREFIX)
    message(FATAL_ERROR "Wrong parameters for function.")
  endif()
  set(options)
  set(oneValueArgs VERSION)
  set(multiValueArgs COMPILERS FEATURES)
  cmake_parse_arguments(_WCD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT _WCD_COMPILERS OR NOT _WCD_FEATURES)
    message(FATAL_ERROR "Invalid arguments.")
  endif()

  if(_WCD_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments: ${_WCD_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT _WCD_VERSION)
    set(_WCD_VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})
  endif()
#   if (_WCD_VERSION VERSION_LESS 3.0.0) # Version which introduced this function
#     message(FATAL_ERROR "VERSION parameter too low.")
#   endif()

  set(ordered_compilers
    # Order is relevant here. We need to list the compilers which pretend to
    # be GNU/MSVC before the actual GNU/MSVC compiler.
    Clang
    GNU
    MSVC
  )
  foreach(_comp ${_WCD_COMPILERS})
    list(FIND ordered_compilers ${_comp} idx)
    if (idx EQUAL -1)
      message(FATAL_ERROR "Unsupported compiler ${_comp}.")
    endif()
  endforeach()

  file(WRITE ${file_arg} "
// This is a generated file. Do not edit!

#ifndef ${prefix_arg}_COMPILER_DETECTION_H
#define ${prefix_arg}_COMPILER_DETECTION_H

#if !defined (__clang__)
#define __has_extension(ext) 0
#endif
")

  foreach(_lang CXX)

    foreach(ordered_compiler ${ordered_compilers})
      list(FIND _WCD_COMPILERS ${ordered_compiler} idx)
      if (NOT idx EQUAL -1)
        _load_compiler_variables(${ordered_compiler} ${_lang} ${_WCD_FEATURES})
      endif()
    endforeach()

    if(_lang STREQUAL CXX)
      file(APPEND "${file_arg}" "\n#ifdef __cplusplus\n")
    endif()
    foreach(feature ${_WCD_FEATURES})
      list(FIND CMAKE_${_lang}_KNOWN_FEATURES ${feature} idx)
      if (NOT idx EQUAL -1)
        set(_define_check "\n#define ${prefix_arg}_${CMAKE_PP_NAME_${feature}} 0\n")
        if (_cmake_feature_tests_${feature} STREQUAL "1")
          set(_define_check "\n#define ${prefix_arg}_${CMAKE_PP_NAME_${feature}} 1\n")
        elseif (_cmake_feature_tests_${feature})
          string(REPLACE ";" " \\\n    || " _cmake_feature_tests_${feature} "${_cmake_feature_tests_${feature}}")
          set(_define_check "\n#if ${_cmake_feature_tests_${feature}}\n#define ${prefix_arg}_${CMAKE_PP_NAME_${feature}} 1\n#else${_define_check}#endif\n")
        endif()
        file(APPEND "${file_arg}" "${_define_check}")
      endif()
    endforeach()
    if(_lang STREQUAL CXX)
      file(APPEND "${file_arg}" "\n#endif\n")
    endif()

  endforeach()

  file(APPEND ${file_arg} "\n#endif\n")
endfunction()
