
enable_language(C)

set(CMAKE_LINKER_TYPE LLD)

add_executable(main main.c)

if(CMake_TEST_CUDA)
  enable_language(CUDA)

  add_executable(mainCU main.cu)
endif()

if(CMake_TEST_Swift)
  enable_language(Swift)
  add_executable(mainSwift main.swift)
endif()

#
# Generate file for validation
#
if (CMAKE_C_USING_LINKER_MODE STREQUAL "TOOL")
  cmake_path(GET CMAKE_C_USING_LINKER_LLD FILENAME LINKER_TYPE_OPTION)
else()
  set(LINKER_TYPE_OPTION "${CMAKE_C_USING_LINKER_LLD}")
endif()
if(CMake_TEST_CUDA)
  if (CMAKE_CUDA_USING_LINKER_MODE STREQUAL "TOOL")
    cmake_path(GET CMAKE_CUDA_USING_LINKER_LLD FILENAME CUDA_LINKER)
  else()
    set(CUDA_LINKER "${CMAKE_CUDA_USING_LINKER_LLD}")
  endif()
  string(APPEND LINKER_TYPE_OPTION "|${CUDA_LINKER}")
endif()
if(CMake_TEST_Swift)
  if(CMAKE_Swift_USING_LINKER_MODE STREQUAL "TOOL")
    cmake_path(GET CMAKE_Swift_USING_LINKER_LLD FILENAME LINKER_TYPE_OPTION)
  else()
    set(Swift_LINKER "${CMAKE_Swift_USING_LINKER_LLD}")
  endif()
  string(APPEND LINKER_TYPE_OPTION "|${Swift_LINKER}")
endif()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/LINKER_TYPE_OPTION.cmake"
  "set(LINKER_TYPE_OPTION \"${LINKER_TYPE_OPTION}\")\n")
