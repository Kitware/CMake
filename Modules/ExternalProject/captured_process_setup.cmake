# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

if(quiet)
  set(capture_output
    OUTPUT_VARIABLE out_var
    ERROR_VARIABLE  out_var
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )
  set(capture_error_only
    ERROR_VARIABLE  out_var
    ERROR_STRIP_TRAILING_WHITESPACE
  )
else()
  unset(capture_output)
  unset(capture_error_only)
endif()

set(out_var "")
set(accumulated_output "")

macro(_ep_message_quiet_capture mode)
  if("${mode}" STREQUAL "FATAL_ERROR")
    string(JOIN "" detail "${ARGN}")
    if(NOT detail STREQUAL "" AND NOT accumulated_output STREQUAL "")
      string(PREPEND detail "\n")
    endif()
    message(FATAL_ERROR "${accumulated_output}${detail}")
  endif()

  if(quiet)
    if("${mode}" MATCHES "WARNING")
      # We can't provide the full CMake backtrace, but we can at least record
      # the warning message with a sensible prefix
      string(APPEND accumulated_output "${mode}: ")
    endif()
    string(APPEND accumulated_output "${ARGN}\n")
  else()
    message(${mode} ${ARGN})
  endif()
endmacro()

macro(_ep_accumulate_captured_output)
  if(NOT "${out_var}" STREQUAL "")
    string(APPEND accumulated_output "${out_var}\n")
  endif()
endmacro()

macro(_ep_command_check_result result)
  _ep_accumulate_captured_output()
  if(result)
    _ep_message_quiet_capture(FATAL_ERROR ${ARGN})
  endif()
endmacro()
