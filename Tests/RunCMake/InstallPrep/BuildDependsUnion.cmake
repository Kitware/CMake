# An install(... BUILD_DEPENDS) entry is evaluated for each configuration, and
# the rule depends on the union of the results, so preparing for any one
# configuration also prepares what the other configurations' entries name.
# Each dependency records that it was built.
set(union_debug_built "${CMAKE_CURRENT_BINARY_DIR}/union-debug-built.txt")
add_custom_target(union_debug
  COMMAND ${CMAKE_COMMAND} -E touch "${union_debug_built}"
  BYPRODUCTS "${union_debug_built}"
)
set(union_release_built "${CMAKE_CURRENT_BINARY_DIR}/union-release-built.txt")
add_custom_target(union_release
  COMMAND ${CMAKE_COMMAND} -E touch "${union_release_built}"
  BYPRODUCTS "${union_release_built}"
)
install(CODE "message(STATUS \"Installing Union\")"
  COMPONENT Union
  BUILD_DEPENDS
    "$<$<CONFIG:Debug>:union_debug>"
    "$<$<CONFIG:Release>:${union_release_built}>"
)
