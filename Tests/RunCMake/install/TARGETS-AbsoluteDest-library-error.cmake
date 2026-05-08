enable_language(C)
add_library(mylib MODULE empty.c)
cmake_diagnostic(SET CMD_INSTALL_ABSOLUTE_DESTINATION SEND_ERROR)
install(TARGETS mylib LIBRARY DESTINATION /absolute/lib)
