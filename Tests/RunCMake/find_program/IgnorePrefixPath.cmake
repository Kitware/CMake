function(assert_eq var value)
  if(NOT "${${var}}" STREQUAL "${value}")
    message(SEND_ERROR "Expected value of ${var}:\n  ${value}\nActual value:\n  ${${var}}")
  endif()
endfunction()

set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)
set(CMAKE_SYSTEM_PROGRAM_PATH)

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/Prefix)
set(_old_CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_SYSTEM_PREFIX_PATH})
set(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_SOURCE_DIR}/SystemPrefix)
set(prog_ROOT
  ${CMAKE_SOURCE_DIR}/Prefix
  ${CMAKE_SOURCE_DIR}/SystemPrefix
  )

set(CMAKE_IGNORE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/Prefix)
set(CMAKE_SYSTEM_IGNORE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/SystemPrefix)
find_program(prog prog)
assert_eq(prog "prog-NOTFOUND")

set(CMAKE_PREFIX_PATH)
set(CMAKE_SYSTEM_PREFIX_PATH ${_old_CMAKE_SYSTEM_PREFIX_PATH})
set(CMAKE_IGNORE_PREFIX_PATH /)
set(CMAKE_FIND_ROOT_PATH
  ${CMAKE_SOURCE_DIR}/Prefix
  ${CMAKE_SOURCE_DIR}/SystemPrefix
  )
find_program(prog2 prog)
assert_eq(prog2 "prog2-NOTFOUND")
