
cmake_diagnostic(SET CMD_DEPRECATED IGNORE)

message(DEPRECATION "This is not issued")

cmake_diagnostic(SET CMD_AUTHOR IGNORE)

message(AUTHOR_WARNING "This is not issued")
