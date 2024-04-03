
enable_language(C)

set(CMAKE_C_USING_LINKER_FOO_C "${CMAKE_C_USING_LINKER_LLD}")

add_executable(main main.c)
set_property(TARGET main PROPERTY LINKER_TYPE "$<$<LINK_LANGUAGE:C>:FOO_C>$<$<LINK_LANGUAGE:CUDA>:FOO_CUDA>")

if(CMake_TEST_CUDA)
  enable_language(CUDA)

  set(CMAKE_CUDA_USING_LINKER_FOO_CUDA "${CMAKE_CUDA_USING_LINKER_LLD}")

  add_executable(mainCU main.cu)
  set_property(TARGET mainCU PROPERTY LINKER_TYPE "$<$<LINK_LANGUAGE:C>:FOO_C>$<$<LINK_LANGUAGE:CUDA>:FOO_CUDA>")
endif()

if(CMake_TEST_Swift)
  enable_language(Swift)

  set(CMAKE_Swift_USING_LINKER_FOO_Swift "${CMAKE_Swift_USING_LINKER_LLD}")
  add_executable(mainSwift main.swift)
  set_property(TARGET mainSwift PROPERTY LINKER_TYPE FOO_Swift)
endif()

#
# Generate file for validation
#
if (CMAKE_C_USING_LINKER_MODE STREQUAL "TOOL")
  cmake_path(GET CMAKE_C_USING_LINKER_FOO_C FILENAME LINKER_TYPE_OPTION)
else()
  set(LINKER_TYPE_OPTION "${CMAKE_C_USING_LINKER_FOO_C}")
endif()
if(CMake_TEST_CUDA)
  if (CMAKE_CUDA_USING_LINKER_MODE STREQUAL "TOOL")
    cmake_path(GET CMAKE_CUDA_USING_LINKER_FOO_CUDA FILENAME CUDA_LINKER)
  else()
    set(CUDA_LINKER "${CMAKE_CUDA_USING_LINKER_FOO_CUDA}")
  endif()
  string(APPEND LINKER_TYPE_OPTION "|${CUDA_LINKER}")
endif()

if(CMake_TEST_Swift)
  if(CMAKE_Swift_USING_LINKER_MODE STREQUAL "TOOL")
    cmake_path(GET CMAKE_Swift_USING_LINKER_FOO_Swift FILENAME Swift_LINKER)
  else()
    set(Swift_LINKER "${CMAKE_Swift_USING_LINKER_FOO_Swift}")
  endif()
  string(APPEND LINKER_TYPE_OPTION "|${Swift_LINKER}")
endif()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/LINKER_TYPE_OPTION.cmake"
  "set(LINKER_TYPE_OPTION \"${LINKER_TYPE_OPTION}\")\n")
