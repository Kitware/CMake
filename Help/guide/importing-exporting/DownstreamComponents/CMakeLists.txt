cmake_minimum_required(VERSION 3.15)
project(DownstreamComponents)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find MathFunctions
find_package(MathFunctions 3.4 COMPONENTS Addition SquareRoot)

# create executable
add_executable(myexe main.cc)

# use MathFunctions library
target_link_libraries(myexe PRIVATE MathFunctions::Addition MathFunctions::SquareRoot)

# Workaround for GCC on AIX to avoid -isystem, not needed in general.
set_property(TARGET myexe PROPERTY NO_SYSTEM_FROM_IMPORTED 1)
