enable_language(C)
set(CMAKE_C_CPPCHECK "${PSEUDO_CPPCHECK}" -error)
add_executable(main main.c)
