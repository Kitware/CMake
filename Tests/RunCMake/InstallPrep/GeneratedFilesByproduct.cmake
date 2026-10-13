enable_language(C)

# A generated file installed by install(FILES) that is not the unique output
# of a target is prepared by depending on the file itself.  Generators that
# cannot do so diagnose it.
set(byproduct "${CMAKE_CURRENT_BINARY_DIR}/byproduct.txt")
add_custom_target(produce_byproduct
  COMMAND ${CMAKE_COMMAND} -E touch "${byproduct}"
  BYPRODUCTS "${byproduct}"
)
install(FILES "${byproduct}"
  DESTINATION share/install-prep
  COMPONENT Byproduct
)

add_library(unit_tests STATIC neverbuild.c)
