enable_language(C)

add_library(runtime_helper STATIC lib.c)
add_executable(runtime EXCLUDE_FROM_ALL runtime.c)
target_link_libraries(runtime PRIVATE runtime_helper)
install(TARGETS runtime
  RUNTIME DESTINATION bin
  COMPONENT Runtime
)

add_library(other_component STATIC neverbuild.c)
install(TARGETS other_component
  ARCHIVE DESTINATION lib
  COMPONENT Other
)

add_library(unit_tests STATIC neverbuild.c)
