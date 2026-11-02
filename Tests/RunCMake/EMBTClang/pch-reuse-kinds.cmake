# One precompiled header, built by a static library and reused by a shared
# library, a module library, a console executable and a GUI executable.  bcc64x
# records some target-kind switches in a precompiled header and rejects a
# mismatch (E904), so they must not reach compilation; each target must still
# see its own __DLL__/__CONSOLE__ state.  The sources #error on the wrong
# state, so a clean build is the assertion.  Driver-gated; a no-op where bcc64x
# is absent.

if(NOT WIN32)
  return()
endif()

set(_hints "")
if(DEFINED ENV{TP})
  list(APPEND _hints "$ENV{TP}/bin64")
endif()
find_program(EMBT_BCC64X NAMES bcc64x HINTS ${_hints})
if(NOT EMBT_BCC64X)
  message(STATUS
    "bcc64x not found on PATH or under $TP; PCH reuse check skipped.")
  return()
endif()

set(_bin "${CMAKE_CURRENT_BINARY_DIR}/pch-reuse-kinds-build")
file(REMOVE_RECURSE "${_bin}")
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/pch-reuse-kinds"
    -B "${_bin}"
    -G "${CMAKE_GENERATOR}"
    "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
    "-DCMAKE_CXX_COMPILER=${EMBT_BCC64X}"
  RESULT_VARIABLE _rc OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
if(NOT _rc EQUAL 0)
  message(SEND_ERROR "configure failed:\n${_o}${_e}")
  return()
endif()
execute_process(
  COMMAND ${CMAKE_COMMAND} --build "${_bin}"
  RESULT_VARIABLE _rc OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
if(NOT _rc EQUAL 0)
  message(SEND_ERROR
    "build failed -- a target could not reuse the precompiled header, or saw "
    "the wrong __DLL__/__CONSOLE__ state:\n${_o}${_e}")
endif()
