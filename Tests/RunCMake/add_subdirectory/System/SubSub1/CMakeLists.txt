add_library(subsub1bar STATIC bar.cpp)
target_include_directories(subsub1bar PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_library(subsub1foo STATIC foo.cpp)
set_target_properties(subsub1foo PROPERTIES SYSTEM OFF)

add_executable(subsub1zot zot.cpp)
