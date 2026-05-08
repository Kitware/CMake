enable_language(C)
add_executable(myexe empty.c)
cmake_diagnostic(SET CMD_INSTALL_ABSOLUTE_DESTINATION SEND_ERROR)
install(TARGETS myexe RUNTIME DESTINATION /absolute/bin)
