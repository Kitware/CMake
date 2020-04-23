file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ExeNoRead" "#!/bin/sh\n")
execute_process(COMMAND chmod -r+x "${CMAKE_CURRENT_BINARY_DIR}/ExeNoRead")
find_program(ExeNoRead_EXECUTABLE NAMES ExeNoRead NO_DEFAULT_PATH PATHS "${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "ExeNoRead_EXECUTABLE='${ExeNoRead_EXECUTABLE}'")
