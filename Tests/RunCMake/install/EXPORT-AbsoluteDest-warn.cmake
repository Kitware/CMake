enable_language(C)
add_library(mylib STATIC empty.c)
install(TARGETS mylib EXPORT myexp DESTINATION lib)
install(EXPORT myexp DESTINATION /absolute/path)
