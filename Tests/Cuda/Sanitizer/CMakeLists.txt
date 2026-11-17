cmake_minimum_required(VERSION 4.3)
project(Sanitizer CXX)

enable_testing()

# Goal for this example:
# Validate that we can use CUDAToolkit sanitizer
find_package(CUDAToolkit REQUIRED)

if(CUDAToolkit_VERSION VERSION_GREATER_EQUAL 10.1)
  add_executable(CudaSanitizer test_sanitizer.cpp)
  target_link_libraries(CudaSanitizer PRIVATE CUDA::sanitizer CUDA::cudart_static)

  add_test(NAME CudaSanitizer COMMAND CudaSanitizer)
  if(WIN32)
    get_property(sanitizer_path TARGET CUDA::sanitizer PROPERTY IMPORTED_LOCATION)
    get_filename_component(sanitizer_dir "${sanitizer_path}" DIRECTORY)
    set_property(TEST CudaSanitizer PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${sanitizer_dir}")
  endif()
endif()
