add_library(gui INTERFACE)
add_library(widget INTERFACE SYMBOLIC)

install(TARGETS gui widget EXPORT gui-targets)
install(EXPORT gui-targets DESTINATION lib/cmake/gui FILE gui-config.cmake NAMESPACE gui::)
