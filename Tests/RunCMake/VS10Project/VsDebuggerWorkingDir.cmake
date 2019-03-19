enable_language(CXX)
add_library(foo foo.cpp)
add_custom_target(bar)

set_target_properties(foo bar PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "my-debugger-directory $<TARGET_PROPERTY:foo,NAME>")
