add_library(testLibDeprecation STATIC empty.cpp)
set_property(TARGET testLibDeprecation PROPERTY DEPRECATION
  "Deprecated version:
  This is a long line of preformatted text that would otherwise wrap to multiple lines.
Please use latest version.")

add_executable(testExe1 empty.cpp)
target_link_libraries(testExe1 testLibDeprecation)
