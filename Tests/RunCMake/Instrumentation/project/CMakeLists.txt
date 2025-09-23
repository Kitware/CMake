cmake_minimum_required(VERSION 3.30)
project(instrumentation)
enable_testing()
if (EXISTS ${INSTRUMENT_COMMAND_FILE})
  set(CMAKE_EXPERIMENTAL_INSTRUMENTATION "a37d1069-1972-4901-b9c9-f194aaf2b6e0")
  include(${INSTRUMENT_COMMAND_FILE})
endif()

add_executable(main main.cxx)
add_library(lib lib.cxx)
target_link_libraries(main lib)
add_custom_command(
  COMMAND ${CMAKE_COMMAND} -E true
  OUTPUT output1 output2
)
set_property(SOURCE output1 output2 PROPERTY SYMBOLIC 1)
add_custom_target(customTarget ALL
  DEPENDS output1
)
add_test(NAME test COMMAND $<TARGET_FILE:main>)
install(TARGETS main)
set_target_properties(main PROPERTIES LABELS "label1;label2")
set_target_properties(lib PROPERTIES LABELS "label3")
