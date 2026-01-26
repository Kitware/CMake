
add_library(foo INTERFACE)

file(GENERATE OUTPUT result.txt CONTENT "$<FILE_SET_PROPERTY:foo,foo,FOO>")
