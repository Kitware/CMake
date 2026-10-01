enable_language(C)
set(CMAKE_VERBOSE_MAKEFILE TRUE)

add_library(iface INTERFACE)
set_target_properties(iface PROPERTIES
  INTERFACE_COMPILE_WRAPPERS NOT_A_COMPILE_WRAPPER
  INTERFACE_LINK_WRAPPERS NOT_A_LINK_WRAPPER
)

add_executable(main main.c)
target_link_libraries(main PRIVATE iface)
