# Error message backtrace points here but call stack shows DEFERRED execution in parent.
cmake_language(DEFER DIRECTORY .. CALL message SEND_ERROR "Deferred Error from Subdirectory")
