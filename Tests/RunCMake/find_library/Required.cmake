list(APPEND CMAKE_FIND_LIBRARY_PREFIXES lib)
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .a)
find_library(LIB_exists
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_DEFAULT_PATH
  REQUIRED
  )
message(STATUS "LIB_exists='${LIB_exists}'")

find_library(LIB_doNotExists
  NAMES doNotExists
  REQUIRED
  )
