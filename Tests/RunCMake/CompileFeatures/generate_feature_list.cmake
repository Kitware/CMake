
enable_language(C)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/c_features.txt"
  "${CMAKE_C_COMPILE_FEATURES}"
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cxx_features.txt"
  "${CMAKE_CXX_COMPILE_FEATURES}"
)

foreach(standard 98 11)
  set(CXX${standard}_FLAG NOTFOUND)
  if (DEFINED CMAKE_CXX${standard}_STANDARD_COMPILE_OPTION)
    set(CXX${standard}_FLAG ${CMAKE_CXX${standard}_STANDARD_COMPILE_OPTION})
  endif()

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cxx${standard}_flag.txt"
    "${CXX${standard}_FLAG}"
  )
  set(CXX${standard}EXT_FLAG NOTFOUND)
  if (DEFINED CMAKE_CXX${standard}_EXTENSION_COMPILE_OPTION)
    set(CXX${standard}EXT_FLAG ${CMAKE_CXX${standard}_EXTENSION_COMPILE_OPTION})
  endif()

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cxx${standard}ext_flag.txt"
    "${CXX${standard}EXT_FLAG}"
  )
endforeach()
