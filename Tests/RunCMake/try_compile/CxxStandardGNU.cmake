enable_language(CXX)
try_compile(result ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CxxStandardGNU.cxx
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED 1
  CXX_EXTENSIONS 0
  OUTPUT_VARIABLE out
  )
if(NOT result)
  message(FATAL_ERROR "try_compile failed:\n${out}")
endif()
