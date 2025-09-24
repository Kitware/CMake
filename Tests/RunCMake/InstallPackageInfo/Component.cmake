add_library(foo INTERFACE)

install(TARGETS foo EXPORT foo DESTINATION .)
install(PACKAGE_INFO foo DESTINATION cps EXPORT foo COMPONENT cps)
get_cmake_property(components COMPONENTS)
message(STATUS "COMPONENTS='${components}'")
