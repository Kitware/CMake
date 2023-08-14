cmake_minimum_required(VERSION 3.18)
project(StaticRuntimePlusToolkit CUDA)

#Goal for this example:
# Validate that with cuda we can use some components of the CUDA toolkit, and
# specify the cuda runtime
find_package(CUDAToolkit REQUIRED)

add_library(Common OBJECT curand.cu nppif.cu)
target_link_libraries(Common PRIVATE CUDA::toolkit)
set_target_properties(Common PROPERTIES POSITION_INDEPENDENT_CODE ON)

#static runtime with shared toolkit libraries
add_library(SharedToolkit SHARED shared.cu)
target_link_libraries(SharedToolkit PRIVATE Common CUDA::curand CUDA::nppif )
set_target_properties(SharedToolkit PROPERTIES CUDA_RUNTIME_LIBRARY none)
target_link_libraries(SharedToolkit PUBLIC CUDA::cudart_static)

# Verify the CUDA Toolkit has static libraries
if(TARGET CUDA::curand_static AND
   TARGET CUDA::nppif_static)
  #static runtime with static toolkit libraries
  add_library(StaticToolkit SHARED static.cu)
  target_compile_definitions(StaticToolkit INTERFACE "HAS_STATIC_VERSION")
  target_link_libraries(StaticToolkit PRIVATE Common CUDA::curand_static CUDA::nppif_static)

  #static runtime with mixed toolkit libraries
  add_library(MixedToolkit SHARED mixed.cu)
  target_compile_definitions(MixedToolkit INTERFACE "HAS_MIXED_VERSION")
  target_link_libraries(MixedToolkit PRIVATE Common CUDA::curand CUDA::nppif_static)
  set_target_properties(MixedToolkit PROPERTIES CUDA_RUNTIME_LIBRARY Static)
endif()

add_executable(CudaOnlyStaticRuntimePlusToolkit main.cu)
target_link_libraries(CudaOnlyStaticRuntimePlusToolkit PRIVATE SharedToolkit
                      $<TARGET_NAME_IF_EXISTS:StaticToolkit>
                      $<TARGET_NAME_IF_EXISTS:MixedToolkit>)

if(UNIX)
  # Help the shared cuda runtime find libcurand and libnppif when they are not located
  # in a default system searched location
  find_package(CUDAToolkit REQUIRED)
  set_property(TARGET CudaOnlyStaticRuntimePlusToolkit PROPERTY BUILD_RPATH "${CUDAToolkit_LIBRARY_DIR}")
endif()
