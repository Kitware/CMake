# C++ module scanning is not supported: a C++20 target that includes standard
# headers must build without being scanned, even when clang-scan-deps is
# present beside bcc64x, and a target with C++ module sources must be rejected
# at configure time.  Driver-gated: a no-op where bcc64x is not installed.
if(NOT WIN32)
  return()
endif()

set(_hints "")
if(DEFINED ENV{TP})
  list(APPEND _hints "$ENV{TP}/bin64" "$ENV{TP}/bin")
endif()
find_program(EMBT_BCC64X NAMES bcc64x HINTS ${_hints})
if(NOT EMBT_BCC64X)
  message(STATUS "bcc64x not found; C++ module scanning check skipped.")
  return()
endif()
get_filename_component(_bindir "${EMBT_BCC64X}" DIRECTORY)

function(_scan_configure case)
  set(_b "${CMAKE_CURRENT_BINARY_DIR}/scan-${case}")
  file(REMOVE_RECURSE "${_b}")
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -S "${CMAKE_CURRENT_SOURCE_DIR}/cxx-module-scanning"
      -B "${_b}"
      -G "${CMAKE_GENERATOR}"
      "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
      "-DCMAKE_CXX_COMPILER=${EMBT_BCC64X}"
      ${ARGN}
    OUTPUT_VARIABLE _out ERROR_VARIABLE _err RESULT_VARIABLE _rc)
  set(_rc "${_rc}" PARENT_SCOPE)
  set(_log "${_out}${_err}" PARENT_SCOPE)
  set(_b "${_b}" PARENT_SCOPE)
endfunction()

# A C++20 target builds without scan steps, also when a scanner is named.
set(_cases plain)
if(EXISTS "${_bindir}/clang-scan-deps.exe")
  list(APPEND _cases scanner)
endif()
foreach(_case IN LISTS _cases)
  set(_opts "")
  if(_case STREQUAL "scanner")
    set(_opts
      "-DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=${_bindir}/clang-scan-deps.exe")
  endif()
  _scan_configure(${_case} ${_opts})
  if(NOT _rc EQUAL 0)
    message(SEND_ERROR "configure (${_case}) failed:\n${_log}")
    continue()
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${_b}"
    OUTPUT_VARIABLE _out ERROR_VARIABLE _err RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(SEND_ERROR "build (${_case}) failed:\n${_out}${_err}")
    continue()
  endif()
  if(CMAKE_GENERATOR MATCHES "Ninja")
    file(GLOB _ninja_files "${_b}/build*.ninja")
    foreach(_f IN LISTS _ninja_files)
      file(STRINGS "${_f}" _steps REGEX "^build [^:]*: CXX_SCAN__")
      if(_steps)
        message(SEND_ERROR "sources are scanned (${_case}):\n${_steps}")
      endif()
    endforeach()
  endif()
endforeach()

# A target with C++ module sources is rejected.
_scan_configure(module -DWITH_MODULE=ON)
set(_want "does not provide a way to discover the import graph dependencies")
string(REGEX REPLACE "[ \n]+" " " _flat "${_log}")
string(FIND "${_flat}" "${_want}" _pos)
if(_rc EQUAL 0 OR _pos EQUAL -1)
  message(SEND_ERROR
    "a CXX_MODULES target should be rejected with:\n  ${_want}\n"
    "Result: ${_rc}\n${_log}")
endif()
