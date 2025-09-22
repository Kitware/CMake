add_library(foo INTERFACE)
install(TARGETS foo EXPORT fooExport)
install(EXPORT fooExport
  DESTINATION "share/cmake"
  COMPONENT "bar"
)
get_cmake_property(components COMPONENTS)
message(STATUS "COMPONENTS='${components}'")
