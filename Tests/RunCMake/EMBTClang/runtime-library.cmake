# EMBT_RUNTIME_LIBRARY must select the linked C++ runtime: Dynamic links the
# shared libc++ (an import of libc++-<ver>.dll), Static (and the default) link it
# statically (no such import).  Build a real exe and module library for each
# value with bcc64x and inspect their imports -- a configure-only check cannot
# tell them apart, since the switches only change the link.  The module library
# must also be compiled with __DLL__, seen as an export only defined under it.
# The helper project also carries a .rc source, so this guards against the
# runtime-library handling mishandling a non-C/CXX language.  Driver-gated: a
# no-op where bcc64x is not installed.
if(NOT WIN32)
  return()
endif()

set(_hints "")
if(DEFINED ENV{TP})
  list(APPEND _hints "$ENV{TP}/bin64" "$ENV{TP}/bin")
endif()
find_program(EMBT_BCC64X NAMES bcc64x HINTS ${_hints})
if(NOT EMBT_BCC64X)
  message(STATUS "bcc64x not found; EMBT_RUNTIME_LIBRARY link check skipped.")
  return()
endif()
get_filename_component(_bindir "${EMBT_BCC64X}" DIRECTORY)
find_program(EMBT_OBJDUMP NAMES llvm-objdump HINTS "${_bindir}")
if(NOT EMBT_OBJDUMP)
  message(STATUS
    "llvm-objdump not found; EMBT_RUNTIME_LIBRARY link check skipped.")
  return()
endif()

# "" exercises the default (Static); Static and Dynamic the explicit values.
foreach(_val "" Static Dynamic)
  if(_val STREQUAL "")
    set(_tag default)
  else()
    set(_tag "${_val}")
  endif()
  set(_b "${CMAKE_CURRENT_BINARY_DIR}/rtl-${_tag}")
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -S "${CMAKE_CURRENT_SOURCE_DIR}/runtime-library"
      -B "${_b}"
      -G "${CMAKE_GENERATOR}"
      "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
      "-DCMAKE_CXX_COMPILER=${EMBT_BCC64X}"
      "-DRTL_VALUE=${_val}"
    OUTPUT_VARIABLE _out ERROR_VARIABLE _err RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(SEND_ERROR "configure (${_tag}) failed:\n${_out}${_err}")
    continue()
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${_b}"
    OUTPUT_VARIABLE _out ERROR_VARIABLE _err RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(SEND_ERROR "build (${_tag}) failed:\n${_out}${_err}")
    continue()
  endif()
  foreach(_art IN ITEMS rtltest.exe rtlmod.dll)
    execute_process(
      COMMAND "${EMBT_OBJDUMP}" -p "${_b}/${_art}"
      OUTPUT_VARIABLE _imports RESULT_VARIABLE _od_rc)
    if(NOT _od_rc EQUAL 0)
      message(SEND_ERROR
        "llvm-objdump (${_tag}, ${_art}) failed:\n${_imports}")
      continue()
    endif()
    if(_imports MATCHES "libc\\+\\+[-0-9]*\\.dll")
      set(_dynamic YES)
    else()
      set(_dynamic NO)
    endif()
    if(_val STREQUAL "Dynamic" AND NOT _dynamic)
      message(SEND_ERROR
        "EMBT_RUNTIME_LIBRARY=Dynamic should link the shared libc++, but "
        "${_art} has no libc++ import.")
    elseif(NOT _val STREQUAL "Dynamic" AND _dynamic)
      message(SEND_ERROR
        "EMBT_RUNTIME_LIBRARY=${_tag} should link libc++ statically, but "
        "${_art} imports the shared libc++.")
    endif()
    if(_art STREQUAL "rtlmod.dll" AND
        NOT _imports MATCHES "rtlmod_has_dll_macro")
      message(SEND_ERROR
        "The module library (${_tag}) was not compiled with __DLL__.")
    endif()
  endforeach()
endforeach()

# An unknown value is rejected when generating for the bcc64x toolchain.
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/runtime-library"
    -B "${CMAKE_CURRENT_BINARY_DIR}/rtl-Bogus"
    -G "${CMAKE_GENERATOR}"
    "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
    "-DCMAKE_CXX_COMPILER=${EMBT_BCC64X}"
    "-DRTL_VALUE=Bogus"
  OUTPUT_VARIABLE _out ERROR_VARIABLE _err RESULT_VARIABLE _rc)
set(_want
  "EMBT_RUNTIME_LIBRARY value 'Bogus' not known for this CXX compiler.")
string(FIND "${_err}" "${_want}" _pos)
if(_rc EQUAL 0 OR _pos EQUAL -1)
  message(SEND_ERROR
    "EMBT_RUNTIME_LIBRARY=Bogus should be rejected with:\n  ${_want}\n"
    "Result: ${_rc}\n${_out}${_err}")
endif()
