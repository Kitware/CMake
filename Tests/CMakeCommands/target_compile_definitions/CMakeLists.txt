
cmake_minimum_required(VERSION 2.8)

project(target_compile_definitions)

add_executable(target_compile_definitions
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_compile_definitions(target_compile_definitions
  PRIVATE MY_PRIVATE_DEFINE
  PUBLIC MY_PUBLIC_DEFINE
  INTERFACE MY_INTERFACE_DEFINE
)

add_executable(consumer
  "${CMAKE_CURRENT_SOURCE_DIR}/consumer.cpp"
)

add_library(linked UNKNOWN IMPORTED)
set_property(TARGET linked PROPERTY
  INTERFACE_COMPILE_DEFINITIONS "MY_LINKED_DEFINE")


target_compile_definitions(consumer
  PRIVATE $<TARGET_PROPERTY:target_compile_definitions,INTERFACE_COMPILE_DEFINITIONS>
  $<$<TARGET_DEFINED:notdefined>:SHOULD_NOT_BE_DEFINED>
  $<$<TARGET_DEFINED:target_compile_definitions>:SHOULD_BE_DEFINED>
  $<LINKED:linked>
  -DDASH_D_DEFINE
)
