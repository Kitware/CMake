cmake_minimum_required(VERSION 3.30)
project(instrumentation)
enable_testing()
if (EXISTS ${INSTRUMENT_COMMAND_FILE})
  include(${INSTRUMENT_COMMAND_FILE})
endif()

add_executable(main main.cxx)
add_library(lib lib.cxx)
target_link_libraries(main lib)
add_custom_command(TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E true
)
add_custom_command(
  COMMAND ${CMAKE_COMMAND} -E true
  OUTPUT output1 output2
)
set_property(SOURCE output1 output2 PROPERTY SYMBOLIC 1)
add_custom_target(customTarget ALL
  COMMAND ${CMAKE_COMMAND} -E true
  DEPENDS output1
)
add_test(NAME test COMMAND $<TARGET_FILE:main>)
install(TARGETS main)
set_target_properties(main PROPERTIES LABELS "label1;label2")
set_target_properties(lib PROPERTIES LABELS "label3")

if (FAIL)
  file(WRITE "${CMAKE_BINARY_DIR}/dummy.cxx"
    "#error \"something which will not compile\"\n"
  )
  add_executable(dummy "${CMAKE_BINARY_DIR}/dummy.cxx")
  add_test(NAME dummy COMMAND ${CMAKE_COMMAND} -E false)
  install(CODE "message(FATAL_ERROR \"Failed install.\")")
endif()
