add_executable(CudaOnlySeparateCompilation main.cu)
target_link_libraries(CudaOnlySeparateCompilation PRIVATE CUDASeparateLibB)
set_target_properties(CudaOnlySeparateCompilation PROPERTIES
  CUDA_STANDARD 11
  CUDA_STANDARD_REQUIRED TRUE
)

if(CMAKE_GENERATOR MATCHES "^Visual Studio")
  # Visual Studio CUDA integration will not perform device linking
  # on a target that itself does not have GenerateRelocatableDeviceCode
  # enabled.
  set_property(TARGET CudaOnlySeparateCompilation PROPERTY CUDA_SEPARABLE_COMPILATION ON)
endif()

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlySeparateCompilation PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
