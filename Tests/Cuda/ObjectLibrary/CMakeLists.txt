cmake_minimum_required(VERSION 3.7)
project (CudaObjectLibrary CUDA CXX)
#Goal for this example:

#build a object files some with cuda and some without than
#embed these into an executable

add_library(CudaMixedObjectLib OBJECT static.cu static.cpp)

add_executable(CudaObjectLibrary
               main.cpp
               $<TARGET_OBJECTS:CudaMixedObjectLib>)
