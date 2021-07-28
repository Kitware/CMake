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

install(TARGETS tgt
  RUNTIME_DEPENDENCIES
  RUNTIME DESTINATION bin COMPONENT bin1
  LIBRARY DESTINATION lib COMPONENT lib1
  FRAMEWORK DESTINATION fw COMPONENT fw1
  )
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  check_components("bin1;fw1;lib1")
else()
  check_components("bin1;lib1")
endif()

install(RUNTIME_DEPENDENCY_SET deps
  RUNTIME DESTINATION bin COMPONENT bin2
  LIBRARY DESTINATION lib COMPONENT lib2
  FRAMEWORK DESTINATION fw COMPONENT fw2
  )
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  check_components("bin1;fw1;fw2;lib1;lib2")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  check_components("bin1;bin2;lib1")
elseif()
  check_components("bin1;lib1;lib2")
endif()
