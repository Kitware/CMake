include(common.cmake)
enable_language(C)

# EXAMPLE_1 is inherited only in Release, EXAMPLE_2 is set only in Debug.

add_library(iface_config INTERFACE)
set_target_properties(iface_config PROPERTIES
  INTERFACE_COMPILE_WRAPPERS "$<$<CONFIG:Release>:EXAMPLE_1>"
  INTERFACE_LINK_WRAPPERS "$<$<CONFIG:Release>:EXAMPLE_1>"
)

add_executable(main main.c)
target_link_libraries(main PRIVATE iface_config)
set_target_properties(main PROPERTIES
  COMPILE_WRAPPERS "$<$<CONFIG:Debug>:EXAMPLE_2>"
  LINK_WRAPPERS "$<$<CONFIG:Debug>:EXAMPLE_2>"
)
