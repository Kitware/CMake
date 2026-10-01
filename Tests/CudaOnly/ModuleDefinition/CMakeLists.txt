cmake_minimum_required(VERSION 3.18)
project(ModuleDefinition CUDA)

if(MSVC)
  # Simulate another language overriding the -DEF: flag.
  # The generators prefer CMAKE_<LANG>_LINK_DEF_FILE_FLAG.
  set(CMAKE_LINK_DEF_FILE_FLAG "bad-def-flag:")
endif()

# Symbols are exported only through the .def file.
add_library(CudaOnlyModuleDefinitionLib SHARED lib.cu lib.def)

add_executable(CudaOnlyModuleDefinition main.cu)
target_link_libraries(CudaOnlyModuleDefinition PRIVATE CudaOnlyModuleDefinitionLib)
