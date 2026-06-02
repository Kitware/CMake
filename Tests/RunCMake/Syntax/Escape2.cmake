macro (escape str)
  message("${str}")
endmacro ()

if(POLICY CMP0219)
  cmake_policy(SET CMP0219 OLD)
endif()

escape("\\")
