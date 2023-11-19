cmake_policy(SET CMP0157 NEW)
enable_language(Swift)
add_executable(E E.swift)
set_target_properties(E PROPERTIES
  WIN32_EXECUTABLE TRUE)
