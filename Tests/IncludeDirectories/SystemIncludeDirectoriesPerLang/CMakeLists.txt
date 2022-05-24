cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(SystemIncludeDirectoriesPerLang)

add_library(c_interface INTERFACE)
set_target_properties(c_interface PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "$<$<COMPILE_LANGUAGE:C>:${CMAKE_CURRENT_SOURCE_DIR}>"
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "$<$<COMPILE_LANGUAGE:C>:${CMAKE_CURRENT_SOURCE_DIR}>"
)
target_compile_options(c_interface INTERFACE "$<$<COMPILE_LANG_AND_ID:C,GNU,Clang>:-Werror=unused-variable>;$<$<COMPILE_LANG_AND_ID:C,MSVC>:/we4101>")

add_library(cxx_interface INTERFACE)
set_target_properties(cxx_interface PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/cxx_system_include>"
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/cxx_system_include>"
)
target_compile_options(cxx_interface INTERFACE "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Werror=unused-variable>;$<$<COMPILE_LANG_AND_ID:C,MSVC>:/we4101>")

# The C header must come before the C++ header for this test to smoke out the
# failure. The order of sources is how CMake determines the include cache
# and we need it to cache on the 'bad' language first
add_executable(consume_multi_lang_includes main.c smoke_out_includes.cxx)
target_link_libraries(consume_multi_lang_includes PRIVATE c_interface cxx_interface)
