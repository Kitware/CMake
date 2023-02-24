enable_language(C)

add_library(exA SHARED dll.c)
set_target_properties(exA PROPERTIES
  SOVERSION 2
  DLL_NAME_WITH_SOVERSION 1
  )

set(CMAKE_DLL_NAME_WITH_SOVERSION 1)
add_library(exB SHARED dll.c)
set_property(TARGET exB PROPERTY SOVERSION 2)

add_custom_target(checkNames ALL
  COMMAND ${CMAKE_COMMAND} -E echo exA_name="$<TARGET_FILE_NAME:exA>"
  COMMAND ${CMAKE_COMMAND} -E echo exB_name="$<TARGET_FILE_NAME:exB>"
  VERBATIM
  )
add_dependencies(checkNames exA exB)
