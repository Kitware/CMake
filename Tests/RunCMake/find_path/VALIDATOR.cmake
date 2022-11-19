
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


find_path(DIR
  NAMES PrefixInPATH.h
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/include
  VALIDATOR check_default
  )
message(STATUS "DIR='${DIR}'")
unset(DIR CACHE)

find_path(DIR
  NAMES PrefixInPATH.h
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/include
  VALIDATOR check_ok
  )
message(STATUS "DIR='${DIR}'")
unset(DIR CACHE)

find_path(DIR
  NAMES PrefixInPATH.h
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/include
  VALIDATOR check_ko
  )
message(STATUS "DIR='${DIR}'")
unset(DIR CACHE)
