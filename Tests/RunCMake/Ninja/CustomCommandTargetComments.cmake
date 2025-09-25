enable_language(C)

add_executable(hello hello.c)
add_custom_command(TARGET hello PRE_BUILD
  COMMENT "pre-build: $<1:genex>"
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:hello>")
add_custom_command(TARGET hello PRE_LINK
  COMMENT "pre-link: $<1:genex>"
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:hello>")
add_custom_command(TARGET hello POST_BUILD
  COMMENT "post-build: $<1:genex>"
  COMMAND "${CMAKE_COMMAND}" -E echo "$<TARGET_FILE:hello>")
