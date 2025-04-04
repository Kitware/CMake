enable_language(Swift)

add_library(L OBJECT L.swift)
set_target_properties(L PROPERTIES
  Swift_MODULE_NAME El
  Swift_MODULE_DIRECTORY El)
