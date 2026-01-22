
enable_language(C)

add_library(foo foo.c)

file(GENERATE OUTPUT result.txt CONTENT "$<SOURCE_PROPERTY:foo.c,TARGET_DIRECTORY:,FOO>")
