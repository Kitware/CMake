cmake_minimum_required(VERSION 3.12)

message("${x}")
math(EXPR x "${x} + 1")
try_compile(result
  "${CMAKE_CURRENT_BINARY_DIR}/try_compile"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  try_compile
  CMAKE_FLAGS -Dx:STRING=${x}
  )

# We put this last to avoid prematurely triggering the recursion limit
project(try_compile NONE)
