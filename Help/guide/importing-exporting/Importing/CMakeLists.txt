cmake_minimum_required(VERSION 3.15)
project(Importing)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(myexe IMPORTED)

# Set imported location
set_property(TARGET myexe PROPERTY
             IMPORTED_LOCATION "../InstallMyExe/bin/myexe")

# Add custom command to create source file
add_custom_command(OUTPUT main.cc COMMAND myexe)

# Use source file
add_executable(mynewexe main.cc)
