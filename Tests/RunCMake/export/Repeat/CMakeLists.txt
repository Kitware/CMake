add_library(bar INTERFACE)
export(TARGETS bar FILE ${CMAKE_BINARY_DIR}/foo.cmake)
