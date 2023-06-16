enable_language(C)

add_library(Gui IMPORTED UNKNOWN)
set_property(TARGET Gui PROPERTY IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/../FrameworkConsumption-build/install/Gui.framework")

add_executable(app main.c)
target_link_libraries(app PRIVATE Gui)
