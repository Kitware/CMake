add_library(doubleexported INTERFACE)
install(TARGETS doubleexported EXPORT exportset)
export(EXPORT exportset
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export1.cmake")
export(EXPORT exportset NAMESPACE test::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export2.cmake")
add_library(exported INTERFACE)
target_link_libraries(exported INTERFACE doubleexported)
export(TARGETS exported FILE "${CMAKE_CURRENT_BINARY_DIR}/exports.cmake")
