include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

enable_language(CXX)

try_compile(result ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/CxxStandardGNU.cxx
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED 1
  CXX_EXTENSIONS 0
  OUTPUT_VARIABLE out
  )

if(NOT result)
  message(FATAL_ERROR "try_compile failed:\n${out}")
endif()

cmake_policy(SET CMP0067 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_EXTENSIONS 0)

try_compile(result ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/CxxStandardGNU.cxx
  OUTPUT_VARIABLE out
  )

if(NOT result)
  message(FATAL_ERROR "try_compile failed:\n${out}")
endif()
