# Make two interface libraries.
add_library(lib1 INTERFACE)
add_library(lib2 INTERFACE)

# Top-level target that depends on both of them.
add_custom_target(top
  COMMAND ${CMAKE_COMMAND} -E echo top
  DEPENDS lib1 lib2)

# Lowest-level utility target that both libraries depend on.
add_custom_target(util
  COMMAND ${CMAKE_COMMAND} -E echo util)
add_dependencies(lib1 util)
add_dependencies(lib2 util)

# The dependency graph computed by cmComputeTargetDepends will include
# an edge directly from 'top' to 'util'. But it should only include
# one copy of it, even though there are two paths via lib1 and lib2.
set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
