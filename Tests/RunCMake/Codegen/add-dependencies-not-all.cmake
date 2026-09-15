add_executable(main
    main.c
)

add_custom_command(
  OUTPUT generated.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
                                   ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  CODEGEN
)

# N.B. h_creator is *not* added to ALL.
add_custom_target(h_creator DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated.h)

# This test will fail if add_dependencies isn't accounted for in the
# codegen build graph
add_dependencies(main h_creator)
