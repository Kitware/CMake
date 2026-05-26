set(FILE_TO_LOCK "${CMAKE_CURRENT_BINARY_DIR}/file.lock")

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -D "FILE_TO_LOCK=${FILE_TO_LOCK}"
        -P "${CMAKE_CURRENT_LIST_DIR}/lock_script_example.cmake"
    COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -D "FILE_TO_LOCK=${FILE_TO_LOCK}"
        -P "${CMAKE_CURRENT_LIST_DIR}/lock_script_include_example.cmake"
    COMMAND_ERROR_IS_FATAL ANY
)
