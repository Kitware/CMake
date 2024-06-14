include(${CMAKE_CURRENT_LIST_DIR}/exit_7_script_included_with_exit.cmake)

message(FATAL_ERROR "The cmake_language(EXIT 7) from include()-d script doesn't work")
