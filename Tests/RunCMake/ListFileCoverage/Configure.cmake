cmake_language(DEFER CALL set deferred 1)
try_compile(tc PROJECT TryCompileProject
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TryCompileProject
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/tc
  )
