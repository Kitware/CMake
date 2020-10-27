cmake_minimum_required(VERSION 3.15)
project(MyExe)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(myexe main.cxx)

# install executable
install(TARGETS myexe)
