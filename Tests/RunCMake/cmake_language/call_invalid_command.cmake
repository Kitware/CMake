
foreach (command IN ITEMS "function" "ENDFUNCTION"
                          "macro" "endMACRO"
                          "if" "elseif" "else" "endif"
                          "while" "endwhile"
                          "foreach" "endforeach")
  execute_process(COMMAND "${CMAKE_COMMAND}" -DCOMMAND=${command}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/CallInvalidCommand.cmake"
    OUTPUT_QUIET ERROR_QUIET
    RESULT_VARIABLE result)
  if (NOT result)
    message (SEND_ERROR "cmake_language(CALL ${command}) unexpectedly successfull.")
  endif()
endforeach()
