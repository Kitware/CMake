enable_language(CXX)

add_library(interface INTERFACE)
install(TARGETS interface EXPORT export)
export(EXPORT export)

add_library(imported IMPORTED INTERFACE)

try_compile(tc "${CMAKE_CURRENT_BINARY_DIR}/tc" "${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp" LINK_LIBRARIES imported)
