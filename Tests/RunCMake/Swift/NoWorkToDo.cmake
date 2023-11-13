cmake_policy(SET CMP0157 NEW)
enable_language(Swift)
add_executable(hello1 hello.swift)
set_target_properties(hello1 PROPERTIES ENABLE_EXPORTS TRUE)

add_executable(hello2 hello.swift)
