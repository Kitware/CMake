enable_language(CXX)
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "$<1:${PSEUDO_IWYU}>" -some -args)
add_executable(main main.cxx)
