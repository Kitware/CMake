cmake_minimum_required(VERSION 3.25.5)

project(SeparateCompilationObjects LANGUAGES CUDA)

add_library(foo OBJECT foo.cu)
set_target_properties(foo PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_library(bar OBJECT bar.cu)
set_target_properties(bar PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_executable(CudaOnlySeparateCompilationTargetObjects main.cu)
set_target_properties(CudaOnlySeparateCompilationTargetObjects PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
target_link_libraries(CudaOnlySeparateCompilationTargetObjects PRIVATE $<TARGET_OBJECTS:foo> bar)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlySeparateCompilationTargetObjects PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
