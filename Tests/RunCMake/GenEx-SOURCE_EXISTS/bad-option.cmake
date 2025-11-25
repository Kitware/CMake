
enable_language(C)

add_library(foo foo.c)

file(GENERATE OUTPUT result.txt CONTENT "$<SOURCE_EXISTS:foo.c,BAD_OPTION>")
