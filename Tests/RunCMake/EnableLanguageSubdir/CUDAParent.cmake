
# With CMP0220 NEW, CUDA enabled in a subdirectory propagates up to this scope.
cmake_policy(SET CMP0220 NEW)

enable_language(CXX)

add_subdirectory(cuda)

add_executable(main cuda-main.cxx)
target_link_libraries(main PRIVATE cuda_lib)

if(APPLE)
  set_property(TARGET main PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
