# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)

if( NOT ( ("${CMAKE_GENERATOR}" MATCHES "Make") OR
          ("${CMAKE_GENERATOR}" MATCHES "Ninja") ) )
  message(FATAL_ERROR "CUDA language not currently supported by \"${CMAKE_GENERATOR}\" generator")
endif()

if(NOT CMAKE_CUDA_COMPILER)
  set(CMAKE_CUDA_COMPILER_INIT NOTFOUND)

  # finally list compilers to try
  if(NOT CMAKE_CUDA_COMPILER_INIT)
    set(CMAKE_CUDA_COMPILER_LIST nvcc)
  endif()

  _cmake_find_compiler(CUDA)
else()
  _cmake_find_compiler_path(CUDA)
endif()

mark_as_advanced(CMAKE_CUDA_COMPILER)

# Build a small source file to identify the compiler.
if(NOT CMAKE_CUDA_COMPILER_ID_RUN)
  set(CMAKE_CUDA_COMPILER_ID_RUN 1)

  list(APPEND CMAKE_CUDA_COMPILER_ID_MATCH_VENDORS NVidia)
  set(CMAKE_CUDA_COMPILER_ID_MATCH_VENDOR_REGEX_NVidia "nvcc: NVIDIA \(R\) Cuda compiler driver")

  set(CMAKE_CXX_COMPILER_ID_TOOL_MATCH_REGEX "\nLd[^\n]*(\n[ \t]+[^\n]*)*\n[ \t]+([^ \t\r\n]+)[^\r\n]*-o[^\r\n]*CompilerIdCXX/(\\./)?(CompilerIdCXX.xctest/)?CompilerIdCXX[ \t\n\\\"]")
  set(CMAKE_CXX_COMPILER_ID_TOOL_MATCH_INDEX 2)

  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(CUDA CUDAFLAGS CMakeCUDACompilerId.cu)

endif()

include(CMakeFindBinUtils)

# configure all variables set in this file
configure_file(${CMAKE_ROOT}/Modules/CMakeCUDACompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeCUDACompiler.cmake
  @ONLY
  )

set(CMAKE_CUDA_COMPILER_ENV_VAR "CUDACXX")
