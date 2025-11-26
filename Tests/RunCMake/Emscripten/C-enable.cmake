enable_language(C)
foreach(var IN ITEMS
    CMAKE_C_BYTE_ORDER
    CMAKE_C_COMPILER_ARCHITECTURE_ID
    CMAKE_C_SIZEOF_DATA_PTR
  )
  message(STATUS "${var}='${${var}}'")
endforeach()
