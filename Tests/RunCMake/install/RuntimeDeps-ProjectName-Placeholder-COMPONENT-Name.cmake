enable_language(C)

function(check_components value)
  get_cmake_property(comp COMPONENTS)
  if(NOT comp STREQUAL value)
    message(FATAL_ERROR "Expected value of COMPONENTS:\n  ${value}\nActual value of COMPONENTS:\n  ${comp}")
  endif()
endfunction()

project(hello_world)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_library(tgt MODULE obj1.c)
  add_library(tgt2 MODULE obj1.c)
else()
  add_executable(tgt main.c)
  add_executable(tgt2 main.c)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "<PROJECT_NAME>")
install(TARGETS tgt
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  )
install(TARGETS tgt2
  RUNTIME DESTINATION bin COMPONENT "other"
  LIBRARY DESTINATION lib COMPONENT "other"
  )
check_components("hello_world;other")
