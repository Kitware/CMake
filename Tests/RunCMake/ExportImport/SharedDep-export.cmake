enable_language(C)

add_library(foo SHARED foo.c)
install(TARGETS foo EXPORT foo)
install(EXPORT foo DESTINATION lib/cmake/foo)
install(FILES foo-config.cmake.in RENAME foo-config.cmake DESTINATION lib/cmake/foo)

add_library(bar SHARED bar.c)
target_link_libraries(bar PRIVATE foo)
# 'foo' only appears in IMPORTED_LINK_DEPENDENT_LIBRARIES, and so is not enforced on import.
install(TARGETS bar EXPORT bar)
install(EXPORT bar DESTINATION lib/cmake/bar)
install(FILES bar-config.cmake.in RENAME bar-config.cmake DESTINATION lib/cmake/bar)
