enable_language(C)

function(check_components value)
  get_cmake_property(comp COMPONENTS)
  if(NOT comp STREQUAL value)
    message(FATAL_ERROR "Expected value of COMPONENTS:\n  ${value}\nActual value of COMPONENTS:\n  ${comp}")
  endif()
endfunction()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_library(tgt MODULE obj1.c)
else()
  add_executable(tgt main.c)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "default_name")
install(TARGETS tgt
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  )
check_components("default_name")
