
# A block() creates a variable scope, so enable_language() could be propagated with NEW behavior
set(CMAKE_POLICY_WARNING_CMP0220 ON)

block()
  enable_language(CXX)
endblock()

if(CMAKE_CXX_COMPILER_LOADED)
  message(FATAL_ERROR
    "enable_language(): language configuration was propagated out of a block() scope with CMP0220 WARN")
endif()
