if(NOT EXISTS "${generatedFile}")
  message(SEND_ERROR "Missing generated file:\n  ${generatedFile}")
endif()

if (CMAKE_HOST_UNIX)
  find_program(STAT_EXECUTABLE NAMES stat)
  if(NOT STAT_EXECUTABLE)
    return()
  endif()

  if (CMAKE_HOST_SYSTEM_NAME MATCHES "FreeBSD")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %Lp "${sourceFile}"
      OUTPUT_VARIABLE output1
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %Lp "${generatedFile}"
      OUTPUT_VARIABLE output2
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
  elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %A "${sourceFile}"
      OUTPUT_VARIABLE output1
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %A "${generatedFile}"
      OUTPUT_VARIABLE output2
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
  else()
    execute_process(COMMAND "${STAT_EXECUTABLE}" -c %a "${sourceFile}"
      OUTPUT_VARIABLE output1
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
    execute_process(COMMAND "${STAT_EXECUTABLE}" -c %a "${generatedFile}"
      OUTPUT_VARIABLE output2
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
      )
  endif()

  if (NOT output1 EQUAL output2)
    message(SEND_ERROR "generated file has a different permissions source "
          "permissions: \"${output1}\" generated permissions: \"${output2}\"")
  endif()

endif()
