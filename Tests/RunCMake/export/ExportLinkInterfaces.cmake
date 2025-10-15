enable_language(CXX)
add_library(foo empty.cpp)

export(TARGETS foo
  EXPORT_LINK_INTERFACE_LIBRARIES
)
