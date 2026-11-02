# Regression guard for the Embarcadero compiler-id detection: bcc64x must
# identify as EMBTClang while the older Clang-based drivers (bcc32x, bcc64,
# bcc32c) stay Embarcadero.  bcc64x is
# discriminated by __MINGW64__ (bcc64 also defines _WIN64, so _WIN64 would not
# tell them apart), and EMBTClang is tried before Embarcadero because bcc64x
# also defines __BORLANDC__/__CODEGEARC_VERSION__.
#
# Each driver's identity is checked by a nested configure, gated on the driver
# being present: real coverage where a RAD Studio tree is installed, a no-op
# elsewhere (upstream CI has none of these compilers).

if(NOT WIN32)
  return()
endif()

set(_hints "")
if(DEFINED ENV{TP})
  list(APPEND _hints "$ENV{TP}/bin64" "$ENV{TP}/bin")
endif()

# driver name -> expected CMAKE_C_COMPILER_ID
set(_expect
  bcc64x EMBTClang
  bcc32x Embarcadero
  bcc64  Embarcadero
  bcc32c Embarcadero)

set(_found_any NO)
while(_expect)
  list(POP_FRONT _expect _drv _want)
  find_program(EMBT_DRIVER_${_drv} NAMES ${_drv} HINTS ${_hints})
  if(NOT EMBT_DRIVER_${_drv})
    continue()
  endif()
  set(_found_any YES)
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -S "${CMAKE_CURRENT_SOURCE_DIR}/compiler-id"
      -B "${CMAKE_CURRENT_BINARY_DIR}/id-${_drv}"
      -G "${CMAKE_GENERATOR}"
      "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
      "-DCMAKE_C_COMPILER=${EMBT_DRIVER_${_drv}}"
      -DCMAKE_C_COMPILER_WORKS=1
    OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
  if(NOT _out MATCHES "DRIVER_COMPILER_ID=\\[${_want}\\]")
    message(SEND_ERROR
      "driver '${_drv}' should identify as '${_want}'.\n"
      "----- configure output -----\n${_out}${_err}")
  endif()
endwhile()

if(NOT _found_any)
  message(STATUS
    "No Embarcadero drivers found on PATH or under \$TP; "
    "compiler-id regression check skipped.")
endif()
