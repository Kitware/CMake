set(CMAKE_FIND_REQUIRED ON)
find_library(LIB_doNotExists_Optional
  NAMES doNotExists
  OPTIONAL
)
find_library(LIB_doNotExists_OptionalRequired
  NAMES doNotExists
  OPTIONAL
  REQUIRED
)
find_library(LIB_doNotExists
  NAMES doNotExists
)
