enable_language(C)

# A generated header that is not the output of a custom command attached to
# a single target is prepared by depending on the file itself, which runs its
# producer.  Generators that cannot do so instead build the installing target,
# which builds the producer through its dependencies, so its sources must
# compile there.
if(CMAKE_GENERATOR MATCHES "Ninja")
  set(generated_target_source neverbuild.c)
else()
  set(generated_target_source lib.c)
endif()

set(generated_header "${CMAKE_CURRENT_BINARY_DIR}/generated.h")
add_custom_target(produce_header
  COMMAND ${CMAKE_COMMAND} -E touch "${generated_header}"
  BYPRODUCTS "${generated_header}"
)

add_library(generated_headers STATIC EXCLUDE_FROM_ALL
  ${generated_target_source})
add_dependencies(generated_headers produce_header)
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
