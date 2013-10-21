
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

include(${CMAKE_CURRENT_LIST_DIR}/CMakeParseArguments.cmake)

function(_load_compiler_variables CompilerId)
  include("${CMAKE_ROOT}/Modules/Compiler/${CompilerId}-features.cmake" OPTIONAL)
  if (NOT compiler_test_macro)
    message(FATAL_ERROR "Compiler ${CompilerId} does not define compiler_test_macro")
  endif()
  foreach(feature ${ARGN})
    set(x ${feature_tests_${feature}})
    if (feature_test_${feature})
      list(APPEND x "(defined (${compiler_test_macro}) && ${feature_test_${feature}})")
      set(feature_tests_${feature} ${x} PARENT_SCOPE)
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

  if(_WCD_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments: ${_WCD_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT _WCD_VERSION)
    set(_WCD_VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})
  endif()
#   if (_WCD_VERSION VERSION_LESS 3.0.0) # Version which introduced this function
#   if (_WCD_VERSION VERSION_LESS 2.8.12.20131023)
#     message(FATAL_ERROR "VERSION parameter too low.")
#   endif()

  file(WRITE ${file_arg} "
// This is a generated file. Do not edit!

#ifndef ${prefix_arg}_COMPILER_DETECTION_H
#define ${prefix_arg}_COMPILER_DETECTION_H
")

  foreach(ordered_compiler
      # Order is relevant here. We need to list the compilers which pretend to
      # be GNU/MSVC before the actual GNU/MSVC compiler.
      XL
      Intel
      Clang
      GNU
      )
    list(FIND _WCD_COMPILERS ${ordered_compiler} idx)
    if (NOT idx EQUAL -1)
      _load_compiler_variables(${ordered_compiler} ${_WCD_FEATURES})
    endif()
  endforeach()

  foreach(feature ${_WCD_FEATURES})
    set(_define_check "\n#define ${prefix_arg}_${CMAKE_PP_NAME_${feature}} 0\n")
    if (feature_tests_${feature})
      string(REPLACE ";" " \\\n    || " feature_tests_${feature} "${feature_tests_${feature}}")
      set(_define_check "\n#if ${feature_tests_${feature}}\n#define ${prefix_arg}_${CMAKE_PP_NAME_${feature}} 1\n#else${_define_check}#endif\n")
    endif()
    file(APPEND "${file_arg}" "${_define_check}")
  endforeach()

  file(APPEND ${file_arg} "\n#endif\n")
endfunction()
