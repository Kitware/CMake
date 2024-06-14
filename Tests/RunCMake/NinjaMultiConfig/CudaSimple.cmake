cmake_policy(SET CMP0104 NEW)
enable_language(CUDA)
file(TOUCH ${CMAKE_BINARY_DIR}/empty.cmake)

add_library(simplecudaobj OBJECT simplelib.cu)
set_target_properties(simplecudaobj
                      PROPERTIES
                      POSITION_INDEPENDENT_CODE ON)

add_library(simplecudashared SHARED )
target_link_libraries(simplecudashared PRIVATE simplecudaobj)
set_target_properties(simplecudaobj simplecudashared
                      PROPERTIES
                      CUDA_SEPARABLE_COMPILATION ON)

add_executable(simplecudaexe main.cu )
target_link_libraries(simplecudaexe PRIVATE simplecudashared)

if(WIN32 AND CMAKE_CUDA_COMPILER_ID STREQUAL "Clang")
  set(generate_output_files_NO_EXE_LIB 1)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(simplecudaexe simplecudashared simplecudaobj)

file(APPEND "${CMAKE_BINARY_DIR}/target_files.cmake" "set(GENERATED_FILES [==[${CMAKE_BINARY_DIR}/empty.cmake]==])\n")
