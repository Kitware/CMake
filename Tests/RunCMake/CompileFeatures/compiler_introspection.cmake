enable_language(C CXX)

macro(info lang)
  string(APPEND info "\
set(${lang}_STANDARD_DEFAULT ${CMAKE_${lang}_STANDARD_DEFAULT})
set(${lang}_EXTENSIONS_DEFAULT ${CMAKE_${lang}_EXTENSIONS_DEFAULT})
set(${lang}_FEATURES ${CMAKE_${lang}_COMPILE_FEATURES})
")

  foreach(standard ${ARGN})
    string(APPEND info "\
set(${lang}${standard}_FLAG ${CMAKE_${lang}${standard}_STANDARD_COMPILE_OPTION})
set(${lang}${standard}_EXT_FLAG ${CMAKE_${lang}${standard}_EXTENSION_COMPILE_OPTION})
")
  endforeach()
endmacro()

info(C 90 99 11 17 23)
info(CXX 98 11 14 17 20 23)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
