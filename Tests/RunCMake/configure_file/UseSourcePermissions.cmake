configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sourcefile.txt
    ${CMAKE_CURRENT_BINARY_DIR}/sourcefile-use-source-permissions.txt
    USE_SOURCE_PERMISSIONS
)

if (CMAKE_HOST_UNIX)
  find_program(STAT_EXECUTABLE NAMES stat)
  if(NOT STAT_EXECUTABLE)
    return()
  endif()

  if (CMAKE_HOST_SYSTEM_NAME MATCHES "FreeBSD")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %Lp "${CMAKE_CURRENT_SOURCE_DIR}/sourcefile.txt"
      OUTPUT_VARIABLE output1
    )
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %Lp "${CMAKE_CURRENT_BINARY_DIR}/sourcefile-use-source-permissions.txt"
      OUTPUT_VARIABLE output2
    )
  elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %A "${CMAKE_CURRENT_SOURCE_DIR}/sourcefile.txt"
      OUTPUT_VARIABLE output1
    )
    execute_process(COMMAND "${STAT_EXECUTABLE}" -f %A "${CMAKE_CURRENT_BINARY_DIR}/sourcefile-use-source-permissions.txt"
      OUTPUT_VARIABLE output2
    )
  else()
    execute_process(COMMAND "${STAT_EXECUTABLE}" -c %a "${CMAKE_CURRENT_SOURCE_DIR}/sourcefile.txt"
      OUTPUT_VARIABLE output1
    )
    execute_process(COMMAND "${STAT_EXECUTABLE}" -c %a "${CMAKE_CURRENT_BINARY_DIR}/sourcefile-use-source-permissions.txt"
      OUTPUT_VARIABLE output2
    )
  endif()

  if (NOT output1 EQUAL output2)
    message(FATAL_ERROR "configure file has different permissions source "
        "permissions: ${output1} generated permissions: ${output2}")
  endif()

endif()
