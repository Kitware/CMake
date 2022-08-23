include(RunCMake)

run_cmake(no_parameters)
run_cmake(unknown_meta_operation)
foreach(command IN ITEMS
    "function" "ENDFUNCTION"
    "macro" "endMACRO"
    "if" "elseif" "else" "endif"
    "while" "endwhile"
    "foreach" "endforeach"
    "block" "endblock"
    )
  message(STATUS "Running call_invalid_command for ${command}...")
  run_cmake_with_options(call_invalid_command -Dcommand=${command})
endforeach()
run_cmake(call_valid_command)
run_cmake(call_double_evaluation)
run_cmake(call_expanded_command)
run_cmake(call_expanded_command_and_arguments)
run_cmake(call_expanded_command_with_explicit_argument)
run_cmake(call_expand_command_name)
run_cmake(call_expand_function_name)
run_cmake(call_message)
run_cmake(call_message_fatal_error)
run_cmake(call_no_parameters)
run_cmake(call_preserve_arguments)
run_cmake(call_unknown_function)
run_cmake(eval_expand_command_name)
run_cmake(eval_expanded_command_and_arguments)
run_cmake(eval_extra_parameters_between_eval_and_code)
run_cmake(eval_message)
run_cmake(eval_message_fatal_error)
run_cmake(eval_no_code)
run_cmake(eval_no_parameters)
run_cmake(eval_variable_outside_message)
run_cmake(defer_call)
run_cmake(defer_call_add_subdirectory)
run_cmake(defer_call_enable_language)
run_cmake(defer_call_ids)
foreach(command IN ITEMS
    "function" "endfunction"
    "macro" "endmacro"
    "if" "elseif" "else" "endif"
    "while" "endwhile"
    "foreach" "endforeach"
    "block" "endblock"
    "return"
    )
  message(STATUS "Running defer_call_invalid_command for ${command}...")
  run_cmake_with_options(defer_call_invalid_command -Dcommand=${command})
endforeach()
run_cmake(defer_call_invalid_directory)
run_cmake(defer_call_error)
run_cmake(defer_call_missing_directory)
run_cmake(defer_call_policy_PUSH)
run_cmake(defer_call_syntax_error)
run_cmake_with_options(defer_call_trace --trace-expand)
run_cmake_with_options(defer_call_trace_json --trace --trace-format=json-v1)
run_cmake(defer_cancel_call_unknown_argument)
run_cmake(defer_cancel_call_invalid_directory)
run_cmake(defer_cancel_call_id)
run_cmake(defer_cancel_call_id_var)
run_cmake(defer_directory_empty)
run_cmake(defer_directory_missing)
run_cmake(defer_directory_multiple)
run_cmake(defer_id_empty)
run_cmake(defer_id_missing)
run_cmake(defer_id_multiple)
run_cmake(defer_id_var_empty)
run_cmake(defer_id_var_missing)
run_cmake(defer_id_var_multiple)
run_cmake(defer_get_call_ids_missing_var)
run_cmake(defer_get_call_ids_too_many_args)
run_cmake(defer_get_call_ids_invalid_directory)
run_cmake(defer_get_call_ids_id)
run_cmake(defer_get_call_ids_id_var)
run_cmake(defer_get_call_missing_id)
run_cmake(defer_get_call_missing_var)
run_cmake(defer_get_call_too_many_args)
run_cmake(defer_get_call_id_empty)
run_cmake(defer_get_call_unknown_argument)
run_cmake(defer_get_call_id)
run_cmake(defer_get_call_id_var)
run_cmake(defer_missing_arg)
run_cmake(defer_missing_call)
run_cmake(defer_unknown_option)

# Default log level
run_cmake_command(
    get_message_log_level_none
    ${CMAKE_COMMAND}
    -P ${RunCMake_SOURCE_DIR}/get_message_log_level.cmake
  )

# Log level from cache
run_cmake_command(
    get_message_log_level_cache
    ${CMAKE_COMMAND}
    -DCMAKE_MESSAGE_LOG_LEVEL=TRACE
    -P ${RunCMake_SOURCE_DIR}/get_message_log_level.cmake
  )

# Log level from regular variable
run_cmake_command(
    get_message_log_level_var
    ${CMAKE_COMMAND}
    -DNEW_LOG_LEVEL=TRACE
    -P ${RunCMake_SOURCE_DIR}/get_message_log_level.cmake
  )

# Log level from command line
run_cmake_command(
    get_message_log_level_cli
    ${CMAKE_COMMAND}
    --log-level=DEBUG
    -P ${RunCMake_SOURCE_DIR}/get_message_log_level.cmake
  )

# Log level from command line, it has higher priority over a cache variable
run_cmake_command(
    get_message_log_level_cli_and_cache
    ${CMAKE_COMMAND}
    --log-level=DEBUG
    -DCMAKE_MESSAGE_LOG_LEVEL=TRACE
    -P ${RunCMake_SOURCE_DIR}/get_message_log_level.cmake
  )

# Log level from command line, it has higher priority over a regular variable
run_cmake_command(
    get_message_log_level_cli_and_var
    ${CMAKE_COMMAND}
    --log-level=DEBUG
    -DNEW_LOG_LEVEL=TRACE
    -P ${RunCMake_SOURCE_DIR}/get_message_log_level.cmake
  )

# Log level from variable, it has higher priority over a cache variable
run_cmake_command(
    get_message_log_level_var_and_cache
    ${CMAKE_COMMAND}
    -DNEW_LOG_LEVEL=DEBUG
    -DCMAKE_MESSAGE_LOG_LEVEL=TRACE
    -P ${RunCMake_SOURCE_DIR}/get_message_log_level.cmake
  )
