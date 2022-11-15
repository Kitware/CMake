enable_language(C)

add_subdirectory(Scope1)
add_subdirectory(Scope2)

add_library(Scope Scope.c)
target_link_libraries(Scope PRIVATE
  scope1_iface
  scope2_iface
  )

add_custom_target(Custom ALL VERBATIM
  COMMAND ${CMAKE_COMMAND} -E echo "iface scope1: '$<TARGET_PROPERTY:scope1_iface,INTERFACE_COMPILE_DEFINITIONS>'"
  COMMAND ${CMAKE_COMMAND} -E echo "iface scope2: '$<TARGET_PROPERTY:scope2_iface,INTERFACE_COMPILE_DEFINITIONS>'"
  COMMAND ${CMAKE_COMMAND} -E echo "iface scope2 in scope1: '$<TARGET_GENEX_EVAL:scope1_iface,$<TARGET_PROPERTY:scope2_iface,INTERFACE_COMPILE_DEFINITIONS>>'"
  COMMAND ${CMAKE_COMMAND} -E echo "custom scope1: '$<TARGET_GENEX_EVAL:scope1_iface,$<TARGET_PROPERTY:scope1_iface,CUSTOM_PROP>>'"
  COMMAND ${CMAKE_COMMAND} -E echo "custom scope2: '$<TARGET_GENEX_EVAL:scope2_iface,$<TARGET_PROPERTY:scope2_iface,CUSTOM_PROP>>'"
  COMMAND ${CMAKE_COMMAND} -E echo "custom scope2 in scope1: '$<TARGET_GENEX_EVAL:scope1_iface,$<TARGET_PROPERTY:scope2_iface,CUSTOM_PROP>>'"
  )
