find_path(PATH_exists
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_DEFAULT_PATH
  REQUIRED
  )
message(STATUS "PATH_exists='${PATH_exists}'")

find_path(PATH_doNotExists
  NAMES doNotExists.h
  REQUIRED
  )
