enable_language(C)

try_compile(RESULT ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  CMAKE_FLAGS           # no values
  COMPILE_DEFINITIONS   # no values
  LINK_LIBRARIES        # no values
  LINK_OPTIONS          # no values
  )
