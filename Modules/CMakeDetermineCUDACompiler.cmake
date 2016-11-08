# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)
include(${CMAKE_ROOT}/Modules//CMakeParseImplicitLinkInfo.cmake)

if( NOT ( ("${CMAKE_GENERATOR}" MATCHES "Make") OR
          ("${CMAKE_GENERATOR}" MATCHES "Ninja") ) )
  message(FATAL_ERROR "CUDA language not currently supported by \"${CMAKE_GENERATOR}\" generator")
endif()

if(NOT CMAKE_CUDA_COMPILER)
  set(CMAKE_CUDA_COMPILER_INIT NOTFOUND)

    # prefer the environment variable CUDACXX
    if(NOT $ENV{CUDACXX} STREQUAL "")
      get_filename_component(CMAKE_CUDA_COMPILER_INIT $ENV{CUDACXX} PROGRAM PROGRAM_ARGS CMAKE_CUDA_FLAGS_ENV_INIT)
      if(CMAKE_CUDA_FLAGS_ENV_INIT)
        set(CMAKE_CUDA_COMPILER_ARG1 "${CMAKE_CUDA_FLAGS_ENV_INIT}" CACHE STRING "First argument to CXX compiler")
      endif()
      if(NOT EXISTS ${CMAKE_CUDA_COMPILER_INIT})
        message(FATAL_ERROR "Could not find compiler set in environment variable CUDACXX:\n$ENV{CUDACXX}.\n${CMAKE_CUDA_COMPILER_INIT}")
      endif()
    endif()

  # finally list compilers to try
  if(NOT CMAKE_CUDA_COMPILER_INIT)
    set(CMAKE_CUDA_COMPILER_LIST nvcc)
  endif()

  _cmake_find_compiler(CUDA)
else()
  _cmake_find_compiler_path(CUDA)
endif()

mark_as_advanced(CMAKE_CUDA_COMPILER)

#Allow the user to specify a host compiler
set(CMAKE_CUDA_HOST_COMPILER "" CACHE FILEPATH "Host compiler to be used by nvcc")
if(NOT $ENV{CUDAHOSTCXX} STREQUAL "")
  get_filename_component(CMAKE_CUDA_HOST_COMPILER $ENV{CUDAHOSTCXX} PROGRAM)
  if(NOT EXISTS ${CMAKE_CUDA_HOST_COMPILER})
    message(FATAL_ERROR "Could not find compiler set in environment variable CUDAHOSTCXX:\n$ENV{CUDAHOSTCXX}.\n${CMAKE_CUDA_HOST_COMPILER}")
  endif()
endif()

# Build a small source file to identify the compiler.
if(NOT CMAKE_CUDA_COMPILER_ID_RUN)
  set(CMAKE_CUDA_COMPILER_ID_RUN 1)

  # Try to identify the compiler.
  set(CMAKE_CUDA_COMPILER_ID)
  set(CMAKE_CUDA_PLATFORM_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_CUDA_COMPILER_ID_PLATFORM_CONTENT)

  list(APPEND CMAKE_CUDA_COMPILER_ID_MATCH_VENDORS NVIDIA)
  set(CMAKE_CUDA_COMPILER_ID_MATCH_VENDOR_REGEX_NVIDIA "nvcc: NVIDIA \(R\) Cuda compiler driver")

  set(CMAKE_CXX_COMPILER_ID_TOOL_MATCH_REGEX "\nLd[^\n]*(\n[ \t]+[^\n]*)*\n[ \t]+([^ \t\r\n]+)[^\r\n]*-o[^\r\n]*CompilerIdCUDA/(\\./)?(CompilerIdCUDA.xctest/)?CompilerIdCUDA[ \t\n\\\"]")
  set(CMAKE_CXX_COMPILER_ID_TOOL_MATCH_INDEX 2)

  set(CMAKE_CUDA_COMPILER_ID_FLAGS_ALWAYS "-v")
  if(CMAKE_CUDA_HOST_COMPILER)
      list(APPEND CMAKE_CUDA_COMPILER_ID_FLAGS_ALWAYS "-ccbin=${CMAKE_CUDA_HOST_COMPILER}")
  endif()

  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(CUDA CUDAFLAGS CMakeCUDACompilerId.cu)
endif()

include(CMakeFindBinUtils)

#if this compiler vendor is matches NVIDIA we can determine
#what the host compiler is. This only needs to be done if the CMAKE_CUDA_HOST_COMPILER
#has NOT been explicitly set
#
#Find the line from compiler ID that contains a.out ( or last line )
#We also need to find the implicit link lines. Which can be done by replacing
#the compiler with cuda-fake-ld  and pass too CMAKE_PARSE_IMPLICIT_LINK_INFO
if(CMAKE_CUDA_COMPILER_ID STREQUAL NVIDIA)
  #grab the last line of the output which holds the link line
  string(REPLACE "#\$ " ";" nvcc_output "${CMAKE_CUDA_COMPILER_PRODUCED_OUTPUT}")
  list(GET nvcc_output -1 nvcc_output)

  #extract the compiler that is being used for linking
  string(REPLACE " " ";" nvcc_output_to_find_launcher "${nvcc_output}")
  list(GET nvcc_output_to_find_launcher 0 CMAKE_CUDA_HOST_LINK_LAUNCHER)
  #we need to remove the quotes that nvcc adds around the directory section
  #of the path
  string(REPLACE "\"" "" CMAKE_CUDA_HOST_LINK_LAUNCHER "${CMAKE_CUDA_HOST_LINK_LAUNCHER}")

  #prefix the line with cuda-fake-ld so that implicit link info believes it is
  #a link line
  set(nvcc_output "cuda-fake-ld ${nvcc_output}")
  CMAKE_PARSE_IMPLICIT_LINK_INFO("${nvcc_output}"
                                 CMAKE_CUDA_HOST_IMPLICIT_LINK_LIBRARIES
                                 CMAKE_CUDA_HOST_IMPLICIT_LINK_DIRECTORIES
                                 CMAKE_CUDA_HOST_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES
                                 log
                                 "${CMAKE_CUDA_IMPLICIT_OBJECT_REGEX}")

  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
          "Parsed CUDA nvcc implicit link information from above output:\n${log}\n\n")

endif()

# configure all variables set in this file
configure_file(${CMAKE_ROOT}/Modules/CMakeCUDACompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeCUDACompiler.cmake
  @ONLY
  )

set(CMAKE_CUDA_COMPILER_ENV_VAR "CUDACXX")
set(CMAKE_CUDA_HOST_COMPILER_ENV_VAR "CUDAHOSTCXX")
