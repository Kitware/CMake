enable_language(C)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/C_comp.cmake" "set(temp_CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\")\n")
