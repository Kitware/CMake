enable_language(C)

add_executable(exe main.c)
install(TARGETS exe RUNTIME_DEPENDENCY_TARGETS
  EXPORT e
  RUNTIME DESTINATION bin)
