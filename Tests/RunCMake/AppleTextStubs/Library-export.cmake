enable_language(C)

add_library(foo SHARED foo.c)
set_property(TARGET foo PROPERTY ENABLE_EXPORTS TRUE)
set_property(TARGET foo PROPERTY LIBRARY_OUTPUT_DIRECTORY $<CONFIG>)
set_property(TARGET foo PROPERTY ARCHIVE_OUTPUT_DIRECTORY $<CONFIG>)

install(TARGETS foo EXPORT foo DESTINATION "${CMAKE_BINARY_DIR}/$<CONFIG>")
install(EXPORT foo DESTINATION lib/foo NAMESPACE foo-install::)
install(FILES foo-config.cmake.in RENAME foo-config.cmake DESTINATION lib/foo)

export(TARGETS foo NAMESPACE foo-build:: FILE Release/foo.cmake)
