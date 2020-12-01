message(SEND_ERROR "SEND_ERROR")
message(FATAL_ERROR "FATAL_ERROR")
# This shouldn't get printed because the script aborts on FATAL_ERROR
message(SEND_ERROR "reaching the unreachable")
