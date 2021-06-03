find_file(FILE_exists
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (DEFINED CACHE{FILE_exists})
  message(SEND_ERROR "Cache variable defined: FILE_exists")
endif()
message(STATUS "FILE_exists='${FILE_exists}'")


find_file(FILE_doNotExists
  NAMES doNotExists.h
  NO_CACHE
  )
if (DEFINED CACHE{FILE_doNotExists})
  message(SEND_ERROR "Cache variable defined: FILE_doNotExists")
endif()
message(STATUS "FILE_doNotExists='${FILE_doNotExists}'")


cmake_policy(SET CMP0125 OLD)
message(STATUS "")
message(STATUS "Policy CMP0125 = OLD")
file(REMOVE "${CMAKE_BINARY_DIR}/PrefixInPATH.h")

set(FILE_cache "unknown" CACHE FILEPATH "")
find_file(FILE_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{FILE_cache})
  message(SEND_ERROR "Cache variable not defined: FILE_cache")
endif()
message(STATUS "CACHED FILE_cache='$CACHE{FILE_cache}'")
unset(FILE_cache CACHE)
message(STATUS "FILE_cache='${FILE_cache}'")


set(FILE_cache "PrefixInPATH.h" CACHE FILEPATH "")
unset(FILE_cache)
find_file(FILE_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{FILE_cache})
  message(SEND_ERROR "Cache variable not defined: FILE_cache")
endif()
message(STATUS "CACHED FILE_cache='$CACHE{FILE_cache}'")
unset(FILE_cache CACHE)
message(STATUS "FILE_cache='${FILE_cache}'")


set(FILE_cache "PrefixInPATH.h" CACHE FILEPATH "")
unset(FILE_cache)
# simulate cache variable defined in command line
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/include/PrefixInPATH.h" DESTINATION "${CMAKE_BINARY_DIR}")
set_property(CACHE FILE_cache PROPERTY TYPE UNINITIALIZED)
find_file(FILE_cache
  NAMES PrefixInPATH.h
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{FILE_cache})
  message(SEND_ERROR "Cache variable not defined: FILE_cache")
endif()
message(STATUS "CACHED FILE_cache='$CACHE{FILE_cache}'")
unset(FILE_cache CACHE)
message(STATUS "FILE_cache='${FILE_cache}'")


cmake_policy(SET CMP0125 NEW)
message(STATUS "")
message(STATUS "Policy CMP0125 = NEW")
file(REMOVE "${CMAKE_BINARY_DIR}/PrefixInPATH.h")

set(FILE_cache "unknown" CACHE FILEPATH "")
unset(FILE_cache)
find_file(FILE_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{FILE_cache})
  message(SEND_ERROR "Cache variable not defined: FILE_cache")
endif()
message(STATUS "CACHED FILE_cache='$CACHE{FILE_cache}'")
unset(FILE_cache CACHE)
message(STATUS "FILE_cache='${FILE_cache}'")


set(FILE_cache "PrefixInPATH.h" CACHE FILEPATH "")
unset(FILE_cache)
find_file(FILE_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{FILE_cache})
  message(SEND_ERROR "Cache variable not defined: FILE_cache")
endif()
message(STATUS "CACHED FILE_cache='$CACHE{FILE_cache}'")
unset(FILE_cache CACHE)
message(STATUS "FILE_cache='${FILE_cache}'")


set(FILE_cache "PrefixInPATH.h" CACHE FILEPATH "")
unset(FILE_cache)
# simulate cache variable defined in command line
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/include/PrefixInPATH.h" DESTINATION "${CMAKE_BINARY_DIR}")
set_property(CACHE FILE_cache PROPERTY TYPE UNINITIALIZED)
find_file(FILE_cache
  NAMES PrefixInPATH.h
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{FILE_cache})
  message(SEND_ERROR "Cache variable not defined: FILE_cache")
endif()
message(STATUS "CACHED FILE_cache='$CACHE{FILE_cache}'")
unset(FILE_cache CACHE)
message(STATUS "FILE_cache='${FILE_cache}'")
