cmake_policy(SET CMP0140 NEW)

function(f)
  find_package(foo UNWIND_INCLUDE)
endfunction()

function(g)
  set(FUNC_CALLED true)
  set(PrimaryUnwind_FOUND false)
  return(PROPAGATE func_called PrimaryUnwind_FOUND)
endfunction()

set(RunCMake_TEST_FAILED "Failed to observe side effects of function() calls during unwind")

f()
g()

if(FUNC_CALLED)
  set(RunCMake_TEST_FAILED)
endif()
