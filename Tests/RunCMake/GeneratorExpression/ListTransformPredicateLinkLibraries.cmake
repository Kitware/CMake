enable_language(C)

# Evaluate LINK_LIBRARIES transitively (CMP0189, CMake 4.1+); the test dir's
# cmake_minimum_required would otherwise leave this OLD.
cmake_policy(SET CMP0189 NEW)

# A dependency tree with mixed library types:
#   app -> { netlib, plugin };  netlib -> { ssl, zlib }
# netlib, ssl, zlib are STATIC_LIBRARY; plugin is INTERFACE_LIBRARY.
add_library(ssl STATIC empty.c)
add_library(zlib STATIC empty.c)
add_library(plugin INTERFACE)
add_library(netlib STATIC empty.c)
target_link_libraries(netlib PUBLIC ssl zlib)
add_library(app STATIC empty.c)
target_link_libraries(app PRIVATE netlib plugin)

# PREDICATE body reads each element's TYPE via TARGET_PROPERTY, demonstrating
# that the body is evaluated in a target context (context-sensitivity).  Only
# STATIC_LIBRARY targets receive the PREPEND; INTERFACE_LIBRARY targets are
# left unchanged.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/filtered.txt"
  CONTENT "$<LIST:TRANSFORM,$<TARGET_PROPERTY:app,LINK_LIBRARIES>,PREPEND,lib:,PREDICATE,$<STREQUAL:$<TARGET_PROPERTY:$<_0>,TYPE>,STATIC_LIBRARY>>\n")

# Exact reference: direct deps first (netlib, plugin), then netlib's transitive
# deps (ssl, zlib); only the three STATIC targets gain the "lib:" prefix.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/expected.txt"
  CONTENT "lib:netlib;plugin;lib:ssl;lib:zlib\n")
