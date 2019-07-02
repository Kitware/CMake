add_library(notexported INTERFACE)
add_library(exported INTERFACE)
target_link_libraries(exported INTERFACE notexported)
export(TARGETS exported FILE "${CMAKE_CURRENT_BINARY_DIR}/exports.cmake")
