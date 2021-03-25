enable_language(CXX)
add_library(foo SHARED empty.cpp)
install(TARGETS foo EXPORT fooExport
  RUNTIME DESTINATION bin
  LIBRARY
     DESTINATION lib
     COMPONENT runtime
     NAMELINK_SKIP
)
install(TARGETS foo EXPORT fooExport
  LIBRARY
     DESTINATION lib
     COMPONENT development
     NAMELINK_ONLY
)
export(EXPORT fooExport FILE "${CMAKE_CURRENT_BINARY_DIR}/foo.cmake")
