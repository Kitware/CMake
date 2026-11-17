cmake_minimum_required(VERSION 3.15)
project(SharedRuntimePlusToolkit CUDA)

#Goal for this example:
# Validate that with c++ we can use some components of the CUDA toolkit, and
# specify the cuda runtime
find_package(CUDAToolkit REQUIRED)

add_library(Common OBJECT curand.cu nppif.cu)
target_link_libraries(Common PRIVATE CUDA::toolkit)
set_target_properties(Common PROPERTIES POSITION_INDEPENDENT_CODE ON)

#shared runtime with shared toolkit libraries
add_library(SharedToolkit SHARED shared.cu)
target_link_libraries(SharedToolkit PRIVATE Common PUBLIC CUDA::curand CUDA::nppif)
set_target_properties(SharedToolkit PROPERTIES CUDA_RUNTIME_LIBRARY none)
target_link_libraries(SharedToolkit PUBLIC CUDA::cudart)

# Verify the CUDA Toolkit has static libraries
if(TARGET CUDA::curand_static AND
   TARGET CUDA::nppif_static)
  #shared runtime with static toolkit libraries
  add_library(StaticToolkit SHARED static.cu)
  target_compile_definitions(StaticToolkit INTERFACE "HAS_STATIC_VERSION")
  target_link_libraries(StaticToolkit PRIVATE Common CUDA::curand_static CUDA::nppif_static)
  set_target_properties(StaticToolkit PROPERTIES CUDA_RUNTIME_LIBRARY Shared)

  #shared runtime with mixed toolkit libraries
  add_library(MixedToolkit SHARED mixed.cu)
  target_compile_definitions(MixedToolkit INTERFACE "HAS_MIXED_VERSION")
  target_link_libraries(MixedToolkit PRIVATE Common CUDA::curand_static CUDA::nppif)
  set_target_properties(MixedToolkit PROPERTIES CUDA_RUNTIME_LIBRARY Shared)
endif()

add_executable(CudaOnlySharedRuntimePlusToolkit main.cu)
target_link_libraries(CudaOnlySharedRuntimePlusToolkit PRIVATE SharedToolkit
                      $<TARGET_NAME_IF_EXISTS:StaticToolkit>
                      $<TARGET_NAME_IF_EXISTS:MixedToolkit>)

if(UNIX)
  # Help the shared cuda runtime find libcudart as it is not located
  # in a default system searched location
  find_package(CUDAToolkit REQUIRED)
  set_property(TARGET CudaOnlySharedRuntimePlusToolkit PROPERTY BUILD_RPATH "${CUDAToolkit_LIBRARY_DIR}")
endif()
