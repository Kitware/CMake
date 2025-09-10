enable_language(C)

set(info "")

# Forward information about the C++ compile features.
string(APPEND info "\
set(CMAKE_C_OUTPUT_EXTENSION \"${CMAKE_C_OUTPUT_EXTENSION}\")
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
