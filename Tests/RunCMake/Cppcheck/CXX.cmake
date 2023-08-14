enable_language(CXX)
set(CMAKE_CXX_CPPCHECK "$<1:${PSEUDO_CPPCHECK}>")
add_executable(main main.cxx)
