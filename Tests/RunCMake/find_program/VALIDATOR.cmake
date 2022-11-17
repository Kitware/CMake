
function(CHECK_DEFAULT result filename)
  message("CHECK='${filename}'")
endfunction()

function(CHECK_OK result filename)
  message("CHECK='${filename}'")
  set(${result} TRUE PARENT_SCOPE)
endfunction()

function(CHECK_KO result filename)
  message("CHECK='${filename}'")
  set(${result} FALSE PARENT_SCOPE)
endfunction()


find_program(PROG
  NAMES testA
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/A
  VALIDATOR check_default
  )
message(STATUS "PROG='${PROG}'")
unset(PROG CACHE)

find_program(PROG
  NAMES testA
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/A
  VALIDATOR check_ok
  )
message(STATUS "PROG='${PROG}'")
unset(PROG CACHE)

find_program(PROG
  NAMES testA
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/A
  VALIDATOR check_ko
  )
message(STATUS "PROG='${PROG}'")
unset(PROG CACHE)
