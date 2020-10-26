cmake_policy(SET CMP0076 NEW)
enable_language(C)

# Test that an interface library can have PUBLIC sources.
# This causes the target to appear in the build system
# *and* causes consumers to use the source.
add_library(iface INTERFACE)
target_sources(iface
  PUBLIC iface.c
  # Private sources do not compile here or propagate.
  PRIVATE iface_broken.c
  )

# Test that an intermediate interface library does not get the
# sources and does not appear in the build system.
add_library(iface2 INTERFACE)
target_link_libraries(iface2 INTERFACE iface)

add_executable(use_iface use_iface.c)
target_link_libraries(use_iface PRIVATE iface2)
