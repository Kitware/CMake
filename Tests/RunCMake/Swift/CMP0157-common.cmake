enable_language(Swift)

add_executable(greetings_default hello.swift)

add_executable(greetings_wmo hello.swift)
set_target_properties(greetings_wmo PROPERTIES
  Swift_COMPILATION_MODE "wholemodule")

add_executable(greetings_incremental hello.swift)
set_target_properties(greetings_incremental PROPERTIES
  Swift_COMPILATION_MODE "incremental")

add_executable(greetings_singlefile hello.swift)
set_target_properties(greetings_singlefile PROPERTIES
  Swift_COMPILATION_MODE "singlefile")

add_executable(greetings_who_knows hello.swift)
set_target_properties(greetings_who_knows PROPERTIES
  Swift_COMPILATION_MODE "not-a-real-mode")
