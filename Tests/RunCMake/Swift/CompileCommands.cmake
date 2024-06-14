if(POLICY CMP0157)
  cmake_policy(SET CMP0157 NEW)
endif()
set(CMAKE_Swift_COMPILATION_MODE "singlefile")

enable_language(Swift)

add_library(CompileCommandLib STATIC E.swift L.swift)
set_target_properties(CompileCommandLib PROPERTIES EXPORT_COMPILE_COMMANDS YES)
