
cmake_minimum_required(VERSION 2.8)

project(target_compile_options)

add_executable(target_compile_options
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_compile_options(target_compile_options
  PRIVATE $<$<CXX_COMPILER_ID:GNU>:-DMY_PRIVATE_DEFINE>
  PUBLIC $<$<CXX_COMPILER_ID:GNU>:-DMY_PUBLIC_DEFINE>
  INTERFACE $<$<CXX_COMPILER_ID:GNU>:-DMY_INTERFACE_DEFINE>
)

add_executable(consumer
  "${CMAKE_CURRENT_SOURCE_DIR}/consumer.cpp"
)

target_compile_options(consumer
  PRIVATE $<$<CXX_COMPILER_ID:GNU>:$<TARGET_PROPERTY:target_compile_options,INTERFACE_COMPILE_OPTIONS>>
)
