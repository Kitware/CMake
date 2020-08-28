cmake_minimum_required(VERSION 3.18)
project(MathFunctions HIP)

# This test covers these major HIP language/runtime requirements:
#
# 1. This makes sure CMake properly specifies the internal clang header dirs
#    that hold headers needed for overloads of device side functions
#
# 2. This makes sure that all HIP include directories are properly marked as
#    system includes so we don't get the following warnings:
#      replacement function 'operator delete' cannot be declared 'inline'#
#
# 3. This makes sure CMake properly links to all the built-in libraries
#    that hip needs that inject support for __half support
#
add_executable(HIPOnlyMathFunctions main.hip)
target_compile_options(HIPOnlyMathFunctions PRIVATE -Werror)
target_compile_features(HIPOnlyMathFunctions PRIVATE hip_std_14)
