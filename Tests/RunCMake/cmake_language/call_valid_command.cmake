
## check continue() usage
set (VALUE 0)
foreach (i RANGE 1 4)
  set (VALUE "${i}")
  cmake_language (CALL "continue")
  set (VALUE "0")
endforeach()
if (NOT VALUE EQUAL "4")
  message (SEND_ERROR "error on continue() usage.")
endif()


## check break() usage
set (VALUE 0)
foreach (i RANGE 1 4)
  set (VALUE "${i}")
  cmake_language (CALL "break")
  set (VALUE 0)
endforeach()
if (NOT VALUE EQUAL "1")
  message (SEND_ERROR "error on break() usage.")
endif()


## check return() usage in macro
macro (call_return_in_macro)
  cmake_language (CALL "return")
  set (VALUE 1)
endmacro()
function (wrapper)
  call_return_in_macro()
  set (VALUE 1 PARENT_SCOPE)
endfunction()

set (VALUE 0)
wrapper()
if (NOT VALUE EQUAL "0")
  message (SEND_ERROR "error on return() usage in macro.")
endif()

set (VALUE 0)
cmake_language (CALL "wrapper")
if (NOT VALUE EQUAL "0")
  message (SEND_ERROR "error on return() usage in macro.")
endif()

function (wrapper2)
  cmake_language (CALL "call_return_in_macro")
  set (VALUE 1 PARENT_SCOPE)
endfunction()

set (VALUE 0)
wrapper2()
if (NOT VALUE EQUAL "0")
  message (SEND_ERROR "error on return() usage in macro.")
endif()

set (VALUE 0)
cmake_language (CALL "wrapper2")
if (NOT VALUE EQUAL "0")
  message (SEND_ERROR "error on return() usage in macro.")
endif()

## check return() usage in function
function (call_return_in_function)
  cmake_language (CALL "return")
  set (VALUE 1 PARENT_SCOPE)
endfunction()

set (VALUE 0)
call_return_in_function()
if (NOT VALUE EQUAL "0")
  message (SEND_ERROR "error on return() usage in function.")
endif()

set (VALUE 0)
cmake_language (CALL "call_return_in_function")
if (NOT VALUE EQUAL "0")
  message (SEND_ERROR "error on return() usage in function.")
endif()


## check usage of include_guard()
set (GUARD_VALUE 0)
include ("${CMAKE_CURRENT_SOURCE_DIR}/CheckIncludeGuard.cmake")
if (NOT GUARD_VALUE EQUAL "1")
  message (SEND_ERROR "error on include_guard() on first include.")
endif()

set (GUARD_VALUE 0)
include ("${CMAKE_CURRENT_SOURCE_DIR}/CheckIncludeGuard.cmake")
if (NOT GUARD_VALUE EQUAL "0")
  message (SEND_ERROR "error on include_guard() on second include.")
endif()


## check usage of cmake_minimum_required() and project()
add_subdirectory (CheckProject)
