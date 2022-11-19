
cmake_policy(SET CMP0140 NEW)

function(FUNC)
  set(VAR "set")
  return(PROPAGATE VAR)
endfunction()

set(VAR "initial")
func()
if (NOT DEFINED VAR OR NOT VAR  STREQUAL "set")
  message(FATAL_ERROR "return(PROPAGATE) not handled correctly.")
endif()
