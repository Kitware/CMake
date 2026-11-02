# Regression guard for C3: a console target is compiled with __CONSOLE__, which
# -tC defines in the IDE build, not only linked with -tC, and a GUI target is
# not -- and compile uses the same per-config WIN32_EXECUTABLE determination as
# the link, so a genex-valued property cannot make the two disagree.  The
# sources #error on the wrong __CONSOLE__ state, so a clean build is the
# assertion.  Driver-gated; a no-op where bcc64x is absent.

if(NOT WIN32)
  return()
endif()

set(_hints "")
if(DEFINED ENV{TP})
  list(APPEND _hints "$ENV{TP}/bin64")
endif()
find_program(EMBT_BCC64X NAMES bcc64x HINTS ${_hints})
if(NOT EMBT_BCC64X)
  message(STATUS "bcc64x not found on PATH or under $TP; __CONSOLE__ check skipped.")
  return()
endif()

function(_subsystem_build case config)
  set(_bin "${CMAKE_CURRENT_BINARY_DIR}/${case}")
  file(REMOVE_RECURSE "${_bin}")
  set(_cfg_arg "")
  if(config)
    set(_cfg_arg "-DCMAKE_BUILD_TYPE=${config}")
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -S "${CMAKE_CURRENT_SOURCE_DIR}/subsystem-macros"
      -B "${_bin}"
      -G "${CMAKE_GENERATOR}"
      "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
      "-DCMAKE_CXX_COMPILER=${EMBT_BCC64X}"
      "-DCASE=${case}"
      ${_cfg_arg}
    RESULT_VARIABLE _rc OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
  if(NOT _rc EQUAL 0)
    message(SEND_ERROR "CASE ${case}: configure failed:\n${_o}${_e}")
    return()
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${_bin}"
    RESULT_VARIABLE _rc OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
  if(NOT _rc EQUAL 0)
    message(SEND_ERROR
      "CASE ${case}: build failed -- a target compiled with the wrong "
      "__CONSOLE__ state:\n${_o}${_e}")
  endif()
endfunction()

# Plain bool: console gets __CONSOLE__, GUI does not.
_subsystem_build(plain "")
# Genex WIN32_EXECUTABLE ($<CONFIG:Release>): GUI in a Release build, so it must
# not get __CONSOLE__ -- catches a non-genex read that would pick console.
_subsystem_build(genex "Release")
