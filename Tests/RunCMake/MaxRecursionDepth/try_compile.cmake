try_compile(result
  "${CMAKE_CURRENT_BINARY_DIR}/try_compile"
  "${CMAKE_CURRENT_SOURCE_DIR}/try_compile"
  try_compile
  CMAKE_FLAGS -Dx:STRING=3
  )
