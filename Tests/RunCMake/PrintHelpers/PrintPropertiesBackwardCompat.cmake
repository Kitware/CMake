include(CMakePrintHelpers)

add_library(foo INTERFACE)
set_property(TARGET foo PROPERTY MY_PROP "val")

cmake_print_properties(TARGETS foo PROPERTIES MY_PROP NOT_SET)
