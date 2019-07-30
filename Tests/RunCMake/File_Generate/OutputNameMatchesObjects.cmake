enable_language(CXX)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>/somefile.cpp"
  CONTENT "static const char content[] = \"$<TARGET_OBJECTS:foo>\";\n"
)

add_library(foo INTERFACE )
