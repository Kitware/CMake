cmake_minimum_required(VERSION 3.26...3.28)
project(req_private_other_target CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(lib)
target_sources(lib
  PUBLIC FILE_SET pub TYPE CXX_MODULES FILES lib.cxx
  PRIVATE FILE_SET priv TYPE CXX_MODULES FILES priv.cxx
)
target_compile_features(lib PUBLIC cxx_std_20)

add_executable(exe main.cxx)
target_link_libraries(exe PRIVATE lib)

add_test(NAME exe COMMAND exe)
