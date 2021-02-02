file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/main.cpp"
  CONTENT "int main() { return 0; }\n"
  NEWLINE_STYLE
  )
