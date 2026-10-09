set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(SYCL)

# Remove gate when IntelLLVM -fsycl-link bug is fixed
if(NOT CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_ON OR
   CMAKE_SYCL_COMPILER_ID STREQUAL "IntelLLVM")
  file(WRITE "${CMAKE_BINARY_DIR}/target_files.cmake" "set(SYCL_SIMPLE_SUPPORTED FALSE)\n")
  return()
endif()
file(TOUCH ${CMAKE_BINARY_DIR}/empty.cmake)

add_library(simplesyclobj OBJECT simplelib.sycl)
set_target_properties(simplesyclobj PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(simplesyclshared SHARED)
target_link_libraries(simplesyclshared PRIVATE simplesyclobj)
set_target_properties(simplesyclobj simplesyclshared PROPERTIES
  SYCL_SEPARABLE_COMPILATION ON)
add_executable(simplesyclexe main.sycl)
target_link_libraries(simplesyclexe PRIVATE simplesyclshared)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
set(generate_output_files_NO_EXE_LIB TRUE)
generate_output_files(simplesyclexe simplesyclshared simplesyclobj)
file(APPEND "${CMAKE_BINARY_DIR}/target_files.cmake"
  "set(SYCL_SIMPLE_SUPPORTED TRUE)\nset(GENERATED_FILES [==[${CMAKE_BINARY_DIR}/empty.cmake]==])\n")
