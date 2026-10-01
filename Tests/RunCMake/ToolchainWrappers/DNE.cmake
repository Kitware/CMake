include(common.cmake)
enable_language(C)

add_executable(main main.c)
target_link_libraries(main PRIVATE iface)
set_target_properties(main PROPERTIES
  COMPILE_WRAPPERS NOT_A_COMPILE_WRAPPER
  LINK_WRAPPERS NOT_A_LINK_WRAPPER
)
