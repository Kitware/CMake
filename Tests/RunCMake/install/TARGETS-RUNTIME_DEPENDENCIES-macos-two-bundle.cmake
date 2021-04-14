enable_language(C)

add_executable(exe1 MACOSX_BUNDLE main.c)
add_executable(exe2 MACOSX_BUNDLE main.c)

install(TARGETS exe1 exe2
  RUNTIME_DEPENDENCIES
  BUNDLE DESTINATION bundles
  FRAMEWORK DESTINATION fw
  )
