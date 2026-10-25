add_executable(main
    main.c
)

add_custom_command(
  OUTPUT generated.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
                                   ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  CODEGEN
)
add_custom_target(h_creator DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.h)

# Not marked CODEGEN, so codegen must not run it.
add_custom_command(
  OUTPUT not-codegen.h
  COMMAND ${CMAKE_COMMAND} -E false
)
add_custom_target(middle DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/not-codegen.h)

# Neither middle nor h_creator is in ALL, so codegen must reach h_creator
# through middle without building middle itself.
add_dependencies(main middle)
add_dependencies(middle h_creator)
