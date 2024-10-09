add_library(foo INTERFACE)
target_link_libraries(foo INTERFACE nonexistent::bar)
install(TARGETS foo DESTINATION lib EXPORT foo-targets)
install(EXPORT foo-targets FILE foo-targets.cmake DESTINATION lib)
