enable_language(C)
add_library(mylib STATIC empty.c)
install(TARGETS mylib EXPORT myexp DESTINATION lib)
install(EXPORT_ANDROID_MK myexp DESTINATION /absolute/path)
