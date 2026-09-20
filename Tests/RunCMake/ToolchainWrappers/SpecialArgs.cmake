enable_language(C)
set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(check "${CMAKE_CURRENT_LIST_DIR}/check-special-args.cmake")
set_property(GLOBAL PROPERTY
  CMAKE_COMPILE_WRAPPER_SPECIAL
    "${CMAKE_COMMAND};-P;${check};--;compile;;semi\\;colon;with space"
)
set_property(GLOBAL PROPERTY
  CMAKE_LINK_WRAPPER_SPECIAL
    "${CMAKE_COMMAND};-P;${check};--;link;;semi\\;colon;with space"
)

add_executable(main main.c)
set_target_properties(main PROPERTIES
  COMPILE_WRAPPERS SPECIAL
  LINK_WRAPPERS SPECIAL
)
