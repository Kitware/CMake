cmake_minimum_required(VERSION 3.18)
project (ObjectLibrary CUDA CXX)
#Goal for this example:
#
#Build C++ and CUDA object files and than use them to make an executable
#Make sure that CMake logic to handle object output when multiple files
#with the same name works
add_subdirectory(Conflicts)

add_library(CudaMixedObjectLib OBJECT static.cu static.cpp)

add_executable(CudaObjectLibrary
               main.cpp
               $<TARGET_OBJECTS:CudaMixedObjectLib>
                $<TARGET_OBJECTS:CudaConflicts>)

# Exercise the shared device-link backend with both forms of object input,
# including a host linker selected independently from the device language.
add_library(CudaDeviceObjects OBJECT device.cu)
set_property(TARGET CudaDeviceObjects PROPERTY CUDA_SEPARABLE_COMPILATION ON)
add_executable(CudaLinkedDeviceObjects device_main.cpp)
target_link_libraries(CudaLinkedDeviceObjects PRIVATE CudaDeviceObjects)
set_property(TARGET CudaLinkedDeviceObjects PROPERTY LINKER_LANGUAGE CXX)
set_property(TARGET CudaLinkedDeviceObjects PROPERTY CUDA_SEPARABLE_COMPILATION ON)
add_executable(CudaDirectDeviceObjects device_main.cpp
  $<TARGET_OBJECTS:CudaDeviceObjects>)
set_property(TARGET CudaDirectDeviceObjects PROPERTY LINKER_LANGUAGE CUDA)
set_property(TARGET CudaDirectDeviceObjects PROPERTY CUDA_SEPARABLE_COMPILATION ON)

# Device-link archive inputs must be built before linking, and inserting the
# registration object must preserve the caller/provider host archive order.
add_library(CudaDeviceProvider STATIC device.cu)
set_property(TARGET CudaDeviceProvider PROPERTY CUDA_SEPARABLE_COMPILATION ON)
add_library(CudaDeviceCaller STATIC device_caller.cpp)
set_property(TARGET CudaDeviceCaller PROPERTY CUDA_RESOLVE_DEVICE_SYMBOLS OFF)
target_link_libraries(CudaDeviceCaller PUBLIC CudaDeviceProvider)
add_executable(CudaDeviceArchiveOrder device_archive_main.cpp)
target_link_libraries(CudaDeviceArchiveOrder PRIVATE CudaDeviceCaller)
set_property(TARGET CudaDeviceArchiveOrder PROPERTY LINKER_LANGUAGE CXX)
set_property(TARGET CudaDeviceArchiveOrder PROPERTY CUDA_SEPARABLE_COMPILATION ON)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaObjectLibrary PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
