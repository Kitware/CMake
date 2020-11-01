find_file(FILE_exists
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_DEFAULT_PATH
  REQUIRED
  )
message(STATUS "FILE_exists='${FILE_exists}'")

find_file(FILE_doNotExists
  NAMES doNotExists.h
  REQUIRED
  )
