list(APPEND CMAKE_FIND_LIBRARY_PREFIXES lib)
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .a)

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


find_library(LIB
  NAMES PrefixInPATH
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  VALIDATOR check_default
  )
message(STATUS "LIB='${LIB}'")
unset(LIB CACHE)

find_library(LIB
  NAMES PrefixInPATH
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  VALIDATOR check_ok
  )
message(STATUS "LIB='${LIB}'")
unset(LIB CACHE)

find_library(LIB
  NAMES PrefixInPATH
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  VALIDATOR check_ko
  )
message(STATUS "LIB='${LIB}'")
unset(LIB CACHE)
