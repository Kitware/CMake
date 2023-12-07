cmake_minimum_required(VERSION 3.18)
project(StubRPATH CXX)

#Verify that linking to a stub library doesn't cause an `-rpath` entry

# Needed for `CUDAToolkit_LIBRARY_SEARCH_DIRS`
find_package(CUDAToolkit REQUIRED)

find_library(CUDA_DRIVER_STUB_LIBRARY
  NAMES cuda
  HINTS ${CUDAToolkit_LIBRARY_SEARCH_DIRS}
        ENV CUDA_PATH
  PATH_SUFFIXES lib64/stubs lib/x64/stubs lib/stubs stubs
)
add_library(imported_stub IMPORTED SHARED)
set_target_properties(imported_stub PROPERTIES IMPORTED_IMPLIB "${CUDA_DRIVER_STUB_LIBRARY}")
set_target_properties(imported_stub PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CUDAToolkit_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 11)
add_executable(StubRPATH main.cxx)
target_link_libraries(StubRPATH PRIVATE imported_stub)
