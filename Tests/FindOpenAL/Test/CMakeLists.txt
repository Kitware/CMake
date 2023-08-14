cmake_minimum_required(VERSION 3.21)
project(TestFindOpenAL CXX)
include(CTest)

find_package(OpenAL REQUIRED)

add_executable(test_tgt main.cxx)
target_link_libraries(test_tgt OpenAL::OpenAL)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.cxx)
target_include_directories(test_var PRIVATE ${OPENAL_INCLUDE_DIR})
target_link_libraries(test_var PRIVATE ${OPENAL_LIBRARY})
add_test(NAME test_var COMMAND test_var)

# OpenAL has been deprecated on macOS since Catalina (10.15)
if(APPLE)
  target_compile_options(test_tgt PRIVATE "-Wno-deprecated-declarations")
  target_compile_options(test_var PRIVATE "-Wno-deprecated-declarations")
endif()
