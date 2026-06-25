# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard(GLOBAL)

function(CMAKE_CHECK_SOURCE_RUNS _lang _source _var)
  if(NOT DEFINED "${_var}")

    if(_lang STREQUAL "C")
      set(_lang_textual "C")
      set(_lang_ext "c")
    elseif(_lang STREQUAL "CXX")
      set(_lang_textual "C++")
      set(_lang_ext "cxx")
    elseif(_lang STREQUAL "CUDA")
      set(_lang_textual "CUDA")
      set(_lang_ext "cu")
    elseif(_lang STREQUAL "Fortran")
      set(_lang_textual "Fortran")
      set(_lang_ext "F90")
    elseif(_lang STREQUAL "HIP")
      set(_lang_textual "HIP")
      set(_lang_ext "hip")
    elseif(_lang STREQUAL "OBJC")
      set(_lang_textual "Objective-C")
      set(_lang_ext "m")
    elseif(_lang STREQUAL "OBJCXX")
      set(_lang_textual "Objective-C++")
      set(_lang_ext "mm")
    else()
      message (SEND_ERROR "check_source_runs: ${_lang}: unknown language.")
      return()
    endif()

    get_property (_supported_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    if (NOT _lang IN_LIST _supported_languages)
      message (SEND_ERROR "check_source_runs: ${_lang}: needs to be enabled before use.")
      return()
    endif()

    set(_FAIL_REGEX)
    set(_SRC_EXT)
    set(_key)
    foreach(arg ${ARGN})
      if("${arg}" MATCHES "^(FAIL_REGEX|SRC_EXT)$")
        set(_key "${arg}")
      elseif(_key STREQUAL "FAIL_REGEX")
        list(APPEND _FAIL_REGEX "${arg}")
      elseif(_key STREQUAL "SRC_EXT")
        set(_SRC_EXT "${arg}")
        set(_key "")
      else()
        set(message_type FATAL_ERROR)
        if (_CheckSourceRuns_old_signature)
          set(message_type AUTHOR_WARNING)
        endif ()
        message("${message_type}" "Unknown argument:\n  ${arg}\n")
        unset(message_type)
      endif()
    endforeach()

    if(NOT _SRC_EXT)
      set(_SRC_EXT ${_lang_ext})
    endif()

    if(CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    else()
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LINK_OPTIONS)
    endif()
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LIBRARIES
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LIBRARIES)
    endif()
    if(CMAKE_REQUIRED_LINK_DIRECTORIES)
      set(_CSR_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    else()
      set(_CSR_LINK_DIRECTORIES)
    endif()
    if(CMAKE_REQUIRED_INCLUDES)
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_INCLUDES)
    endif()

    set(_CSR_EXTRA_CMAKE_ARGUMENTS)
    string(REPLACE "\\;" "\\\\;" CMAKE_REQUIRED_FLAGS
      "${CMAKE_REQUIRED_FLAGS}")
    #[[
    cmake_polify(GET CMP0999 _CSR_CMP0999)
    if(_CSR_CMP0999 STREQUAL "NEW")
      # Join multiple list arguments into the space-separated string that we
      # want. This ensures that the entirety of the value gets passed as
      # compile arguments, which is what the user almost surely intended.
      #
      # FIXME(#27901): This needs a policy to be implemented. This will be
      # available in a future version of CMake.
      list(JOIN CMAKE_REQUIRED_FLAGS " " CMAKE_REQUIRED_FLAGS)
    else()
    #]]
      # If CMAKE_REQUIRED_FLAGS contains an unescaped semicolon, anything after
      # gets passed as flags to 'cmake' itself. This is probably not intended,
      # but we preserve it for compatibility.
      set(_CSR_EXTRA_CMAKE_ARGUMENTS "${CMAKE_REQUIRED_FLAGS}")
      list(POP_FRONT _CSR_EXTRA_CMAKE_ARGUMENTS CMAKE_REQUIRED_FLAGS)
      #[[
      if(NOT CMAKE_REQUIRED_FLAGS STREQUAL "")
        cmake_policy(ISSUE_WARNING CMP0999) TODO
      endif()
      #]]
      # There is at least one known instance of users accidentally passing
      # '-W' arguments intended for the compiler as arguments to CMake. Since
      # CMake complains about unknown '-W' arguments starting with CMake 4.4,
      # we need to strip these for compatibility.
      string(REPLACE "\\;" "\\\\;" _CSR_EXTRA_CMAKE_ARGUMENTS
        "${_CSR_EXTRA_CMAKE_ARGUMENTS}")
      list(FILTER _CSR_EXTRA_CMAKE_ARGUMENTS EXCLUDE REGEX "^-W")
    #endif()

    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Performing Test ${_var}")
    endif()
    string(APPEND _source "\n")
    try_run(${_var}_EXITCODE ${_var}_COMPILED
      SOURCE_FROM_VAR "src.${_SRC_EXT}" _source
      COMPILE_DEFINITIONS -D${_var} ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_${_lang}_SOURCE_COMPILES_ADD_LINK_OPTIONS}
      ${CHECK_${_lang}_SOURCE_COMPILES_ADD_LIBRARIES}
      CMAKE_FLAGS
        -DCOMPILE_DEFINITIONS:STRING=${CMAKE_REQUIRED_FLAGS}
        ${_CSR_EXTRA_CMAKE_ARGUMENTS}
      -DCMAKE_SKIP_RPATH:BOOL=${CMAKE_SKIP_RPATH}
      "${CHECK_${_lang}_SOURCE_COMPILES_ADD_INCLUDES}"
      "${_CSR_LINK_DIRECTORIES}"
      )
    unset(_CSR_LINK_DIRECTORIES)
    # if it did not compile make the return value fail code of 1
    if(NOT ${_var}_COMPILED)
      set(${_var}_EXITCODE 1)
      set(${_var}_EXITCODE 1 PARENT_SCOPE)
    endif()
    # if the return value was 0 then it worked
    if("${${_var}_EXITCODE}" EQUAL 0)
      set(${_var} 1 CACHE INTERNAL "Test ${_var}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "Success")
      endif()
    else()
      if(CMAKE_CROSSCOMPILING AND "${${_var}_EXITCODE}" MATCHES  "FAILED_TO_RUN")
        set(${_var} "${${_var}_EXITCODE}" PARENT_SCOPE)
      else()
        set(${_var} "" CACHE INTERNAL "Test ${_var}")
      endif()

      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "Failed")
      endif()
    endif()
  endif()
endfunction()
