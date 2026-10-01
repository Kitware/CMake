enable_language(Swift)

add_library(L STATIC L.swift)
set_target_properties(L PROPERTIES Swift_MODULE_NAME CustomModule)
