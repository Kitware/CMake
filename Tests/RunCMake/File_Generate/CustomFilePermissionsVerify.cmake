if(NOT EXISTS "${generatedFile}")
  message(SEND_ERROR "Missing file:\n  ${generatedFile}")
endif()

if (UNIX AND NOT MSYS)
  find_program(STAT_EXECUTABLE NAMES stat)
  if(NOT STAT_EXECUTABLE)
    return()
  endif()

  if (CMAKE_HOST_SYSTEM_NAME MATCHES "FreeBSD")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %Lp "${generatedFile}"
      OUTPUT_VARIABLE output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
  elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %A "${generatedFile}"
      OUTPUT_VARIABLE output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
  else()
    execute_process(COMMAND "${STAT_EXECUTABLE}" -c %a "${generatedFile}"
      OUTPUT_VARIABLE output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
  endif()

  if (NOT output EQUAL "711")
    message(SEND_ERROR "file generate has different permissions source "
          "permissions: \"${output}\" desired permissions: \"711\"")
  endif()

endif()
