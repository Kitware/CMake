cmake_minimum_required(VERSION 3.15)
project(Downstream)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find MathFunctions
find_package(MathFunctions 3.4.1 EXACT)

# create executable
add_executable(myexe main.cc)

# use MathFunctions library
target_link_libraries(myexe PRIVATE MathFunctions::MathFunctions)
