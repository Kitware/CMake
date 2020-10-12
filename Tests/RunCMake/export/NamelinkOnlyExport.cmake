enable_language(CXX)
add_library(foo SHARED empty.cpp)
install(TARGETS foo EXPORT fooExport
  RUNTIME DESTINATION bin
  LIBRARY
     DESTINATION lib
     NAMELINK_ONLY
)
export(EXPORT fooExport FILE "${CMAKE_CURRENT_BINARY_DIR}/foo.cmake")
