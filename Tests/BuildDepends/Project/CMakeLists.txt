project(testRebuild)
add_library(foo STATIC ${testRebuild_BINARY_DIR}/foo.c)
add_executable(bar bar.c)
target_link_libraries(bar foo)
