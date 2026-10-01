cmake_minimum_required(VERSION 3.18)
project(LinkDirectory CUDA)

if(MSVC)
  # Simulate another language overriding the link directory flag.
  # The generators prefer CMAKE_<LANG>_LIBRARY_PATH_FLAG.
  set(CMAKE_LIBRARY_PATH_FLAG "bad-libpath-flag:")
endif()

add_library(CudaOnlyLinkDirectoryLib STATIC lib.cu)
set_target_properties(CudaOnlyLinkDirectoryLib PROPERTIES
  OUTPUT_NAME LinkDirectoryLib
  ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_BINARY_DIR}/lib>"
  )

# Link by name so the linker has to search the link directory.
add_executable(CudaOnlyLinkDirectory main.cu)
target_link_directories(CudaOnlyLinkDirectory PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/lib")
target_link_libraries(CudaOnlyLinkDirectory PRIVATE LinkDirectoryLib)
add_dependencies(CudaOnlyLinkDirectory CudaOnlyLinkDirectoryLib)
