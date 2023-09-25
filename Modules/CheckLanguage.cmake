# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckLanguage
-------------

Check whether a language can be enabled by the :command:`enable_language`
or :command:`project` commands:

.. command:: check_language

  .. code-block:: cmake

    check_language(<lang>)

  Try enabling language ``<lang>`` in a test project and record results
  in the cache:

  :variable:`CMAKE_<LANG>_COMPILER`
    If the language can be enabled, this variable is set to the compiler
    that was found.  If the language cannot be enabled, this variable is
    set to ``NOTFOUND``.

    If this variable is already set, either explicitly or cached by
    a previous call, the check is skipped.

  :variable:`CMAKE_<LANG>_HOST_COMPILER`
    This variable is set when ``<lang>`` is ``CUDA`` or ``HIP``.

    If the check detects an explicit host compiler that is required for
    compilation, this variable will be set to that compiler.
    If the check detects that no explicit host compiler is needed,
    this variable will be cleared.

    If this variable is already set, its value is preserved only if
    :variable:`CMAKE_<LANG>_COMPILER` is also set.
    Otherwise, the check runs and overwrites
    :variable:`CMAKE_<LANG>_HOST_COMPILER` with a new result.
    Note that :variable:`CMAKE_<LANG>_HOST_COMPILER` documents it should
    not be set without also setting
    :variable:`CMAKE_<LANG>_COMPILER` to a NVCC compiler.

For example:

.. code-block:: cmake

  check_language(Fortran)
  if(CMAKE_Fortran_COMPILER)
    enable_language(Fortran)
  else()
    message(STATUS "No Fortran support")
  endif()
#]=======================================================================]

include_guard(GLOBAL)

block(SCOPE_FOR POLICIES)
cmake_policy(SET CMP0126 NEW)

macro(check_language lang)
  if(NOT DEFINED CMAKE_${lang}_COMPILER)
    set(_desc "Looking for a ${lang} compiler")
    message(CHECK_START "${_desc}")
    file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang})

    set(extra_compiler_variables)
    if("${lang}" MATCHES "^(CUDA|HIP)$" AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
      set(extra_compiler_variables "set(CMAKE_CUDA_HOST_COMPILER \\\"\${CMAKE_CUDA_HOST_COMPILER}\\\")")
    endif()

    set(_cl_content
      "cmake_minimum_required(VERSION ${CMAKE_VERSION})
project(Check${lang} ${lang})
file(WRITE \"\${CMAKE_CURRENT_BINARY_DIR}/result.cmake\"
  \"set(CMAKE_${lang}_COMPILER \\\"\${CMAKE_${lang}_COMPILER}\\\")\\n\"
  \"${extra_compiler_variables}\\n\"
  )"
    )

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang}/CMakeLists.txt"
      "${_cl_content}")
    if(CMAKE_GENERATOR_INSTANCE)
      set(_D_CMAKE_GENERATOR_INSTANCE "-DCMAKE_GENERATOR_INSTANCE:INTERNAL=${CMAKE_GENERATOR_INSTANCE}")
    else()
      set(_D_CMAKE_GENERATOR_INSTANCE "")
    endif()
    if(CMAKE_GENERATOR MATCHES "^(Xcode$|Green Hills MULTI$|Visual Studio)")
      set(_D_CMAKE_MAKE_PROGRAM "")
    else()
      set(_D_CMAKE_MAKE_PROGRAM "-DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}")
    endif()
    if(CMAKE_TOOLCHAIN_FILE)
      set(_D_CMAKE_TOOLCHAIN_FILE "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}")
    else()
      set(_D_CMAKE_TOOLCHAIN_FILE "")
    endif()
    execute_process(
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang}
      COMMAND ${CMAKE_COMMAND} . -G ${CMAKE_GENERATOR}
                                 -A "${CMAKE_GENERATOR_PLATFORM}"
                                 -T "${CMAKE_GENERATOR_TOOLSET}"
                                 ${_D_CMAKE_GENERATOR_INSTANCE}
                                 ${_D_CMAKE_MAKE_PROGRAM}
                                 ${_D_CMAKE_TOOLCHAIN_FILE}
      OUTPUT_VARIABLE _cl_output
      ERROR_VARIABLE _cl_output
      RESULT_VARIABLE _cl_result
      )
    include(${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang}/result.cmake OPTIONAL)
    if(CMAKE_${lang}_COMPILER AND "${_cl_result}" STREQUAL "0")
      message(CONFIGURE_LOG
        "${_desc} passed with the following output:\n"
        "${_cl_output}\n")
      set(_CHECK_COMPILER_STATUS CHECK_PASS)
    else()
      set(CMAKE_${lang}_COMPILER NOTFOUND)
      set(_CHECK_COMPILER_STATUS CHECK_FAIL)
      message(CONFIGURE_LOG
        "${_desc} failed with the following output:\n"
        "${_cl_output}\n")
    endif()
    message(${_CHECK_COMPILER_STATUS} "${CMAKE_${lang}_COMPILER}")
    set(CMAKE_${lang}_COMPILER "${CMAKE_${lang}_COMPILER}" CACHE FILEPATH "${lang} compiler")
    mark_as_advanced(CMAKE_${lang}_COMPILER)

    if(CMAKE_${lang}_HOST_COMPILER)
      message(STATUS "Looking for a ${lang} host compiler - ${CMAKE_${lang}_HOST_COMPILER}")
      set(CMAKE_${lang}_HOST_COMPILER "${CMAKE_${lang}_HOST_COMPILER}" CACHE FILEPATH "${lang} host compiler")
      mark_as_advanced(CMAKE_${lang}_HOST_COMPILER)
    endif()

  endif()
endmacro()

endblock()
