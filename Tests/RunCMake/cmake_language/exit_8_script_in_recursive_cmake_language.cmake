cmake_language(EVAL CODE "cmake_language(EXIT 8)")

message(FATAL_ERROR "The cmake_language EVAL of EXIT 8 test doesn't work")
