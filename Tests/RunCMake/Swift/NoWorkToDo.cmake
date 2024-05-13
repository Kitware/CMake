cmake_policy(SET CMP0157 NEW)
enable_language(Swift)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/hello.swift "")

add_executable(hello1 ${CMAKE_CURRENT_BINARY_DIR}/hello.swift)
set_target_properties(hello1 PROPERTIES ENABLE_EXPORTS TRUE)

add_executable(hello2 ${CMAKE_CURRENT_BINARY_DIR}/hello.swift)
