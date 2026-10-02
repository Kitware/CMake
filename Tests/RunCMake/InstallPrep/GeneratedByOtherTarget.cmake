enable_language(C)

# A generated header produced by a custom command attached to another target
# is prepared by running that command, without building the installing target.
set(generated_header "${CMAKE_CURRENT_BINARY_DIR}/generated.h")
add_custom_command(OUTPUT "${generated_header}"
  COMMAND ${CMAKE_COMMAND} -E touch "${generated_header}"
)
add_custom_target(produce_header DEPENDS "${generated_header}")

add_library(generated_headers STATIC EXCLUDE_FROM_ALL neverbuild.c)
set_property(TARGET generated_headers
  PROPERTY PUBLIC_HEADER "${generated_header}")
install(TARGETS generated_headers
  ARCHIVE DESTINATION lib
    COMPONENT Archive
    EXCLUDE_FROM_ALL
  PUBLIC_HEADER DESTINATION include
    COMPONENT Headers
)

add_library(unit_tests STATIC neverbuild.c)
