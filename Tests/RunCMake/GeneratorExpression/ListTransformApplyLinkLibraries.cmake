enable_language(C)

# Evaluate LINK_LIBRARIES transitively (CMP0189, CMake 4.1+); the test dir's
# cmake_minimum_required would otherwise leave this OLD.
cmake_policy(SET CMP0189 NEW)

# A dependency tree, linked PUBLIC so each library's dependencies propagate
# through INTERFACE_LINK_LIBRARIES:
#   app -> { net, audio };  net -> { ssl, zlib };  audio -> codec
add_library(ssl STATIC empty.c)
add_library(zlib STATIC empty.c)
add_library(codec STATIC empty.c)

add_library(net STATIC empty.c)
target_link_libraries(net PUBLIC ssl zlib)
add_library(audio STATIC empty.c)
target_link_libraries(audio PUBLIC codec)

add_library(app STATIC empty.c)
target_link_libraries(app PRIVATE net audio)

# app's LINK_LIBRARIES now evaluates to the transitive closure
# (net;audio;ssl;zlib;codec); map each linked library to its on-disk file name.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ll.txt"
  CONTENT "$<LIST:TRANSFORM,$<TARGET_PROPERTY:app,LINK_LIBRARIES>,APPLY,$<TARGET_FILE_NAME:$<_0>>>\n")

# Exact, platform-independent reference: the explicit per-library expansion in
# the transitive-closure order.  The APPLY output must be byte-identical.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/expected.txt"
  CONTENT "$<TARGET_FILE_NAME:net>;$<TARGET_FILE_NAME:audio>;$<TARGET_FILE_NAME:ssl>;$<TARGET_FILE_NAME:zlib>;$<TARGET_FILE_NAME:codec>\n")
