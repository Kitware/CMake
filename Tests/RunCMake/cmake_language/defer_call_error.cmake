# Error message backtrace points here but call stack shows DEFERRED execution.
cmake_language(DEFER CALL message SEND_ERROR "Deferred Error")
add_subdirectory(defer_call_error)
