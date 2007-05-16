project(testRebuild)
add_library(foo STATIC ${testRebuild_BINARY_DIR}/foo.cxx)
add_executable(bar bar.cxx)
target_link_libraries(bar foo)
