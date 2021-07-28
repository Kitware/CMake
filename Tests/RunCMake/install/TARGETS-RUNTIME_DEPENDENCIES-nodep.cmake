enable_language(C)

add_executable(exe main.c)

install(TARGETS exe
  RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES ".*"
  FRAMEWORK DESTINATION fw
  )
