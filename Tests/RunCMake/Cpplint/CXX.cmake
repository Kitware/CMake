enable_language(CXX)
set(CMAKE_CXX_CPPLINT "$<1:${PSEUDO_CPPLINT}>" --verbose=0 --linelength=80)
add_executable(main main.cxx)
