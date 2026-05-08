enable_language(C)
add_library(mylib STATIC empty.c)
install(TARGETS mylib DESTINATION /absolute/path)
