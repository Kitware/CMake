list(APPEND CMAKE_FIND_LIBRARY_PREFIXES lib)
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .a)

find_library(LIB_exists
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (DEFINED CACHE{LIB_exists})
  message(SEND_ERROR "Cache variable defined: LIB_exists")
endif()
message(STATUS "LIB_exists='${LIB_exists}'")


find_library(LIB_doNotExists
  NAMES doNotExists
  NO_CACHE
  )
if (DEFINED CACHE{LIB_doNotExists})
  message(SEND_ERROR "Cache variable defined: LIB_doNotExists")
endif()
message(STATUS "LIB_doNotExists='${LIB_doNotExists}'")


cmake_policy(SET CMP0125 OLD)
message(STATUS "")
message(STATUS "Policy CMP0125 = OLD")
file(REMOVE "${CMAKE_BINARY_DIR}/libPrefixInPATH.a")

set(LIB_cache "unknown" CACHE FILEPATH "")
find_library(LIB_cache
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{LIB_cache})
  message(SEND_ERROR "Cache variable not defined: LIB_cache")
endif()
message(STATUS "CACHED LIB_cache='$CACHE{LIB_cache}'")
unset(LIB_cache CACHE)
message(STATUS "LIB_cache='${LIB_cache}'")


set(LIB_cache "libPrefixInPATH.a" CACHE FILEPATH "")
unset(LIB_cache)
find_library(LIB_cache
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{LIB_cache})
  message(SEND_ERROR "Cache variable not defined: LIB_cache")
endif()
message(STATUS "CACHED LIB_cache='$CACHE{LIB_cache}'")
unset(LIB_cache CACHE)
message(STATUS "LIB_cache='${LIB_cache}'")


set(LIB_cache "libPrefixInPATH.a" CACHE FILEPATH "")
unset(LIB_cache)
# simulate cache variable defined in command line
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/lib/libPrefixInPATH.a" DESTINATION "${CMAKE_BINARY_DIR}")
set_property(CACHE LIB_cache PROPERTY TYPE UNINITIALIZED)
find_library(LIB_cache
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{LIB_cache})
  message(SEND_ERROR "Cache variable not defined: LIB_cache")
endif()
message(STATUS "CACHED LIB_cache='$CACHE{LIB_cache}'")
unset(LIB_cache CACHE)
message(STATUS "LIB_cache='${LIB_cache}'")


cmake_policy(SET CMP0125 NEW)
message(STATUS "")
message(STATUS "Policy CMP0125 = NEW")
file(REMOVE "${CMAKE_BINARY_DIR}/libPrefixInPATH.a")

set(LIB_cache "unknown" CACHE FILEPATH "")
find_library(LIB_cache
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{LIB_cache})
  message(SEND_ERROR "Cache variable not defined: LIB_cache")
endif()
message(STATUS "CACHED LIB_cache='$CACHE{LIB_cache}'")
unset(LIB_cache CACHE)
message(STATUS "LIB_cache='${LIB_cache}'")


set(LIB_cache "libPrefixInPATH.a" CACHE FILEPATH "")
unset(LIB_cache)
find_library(LIB_cache
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{LIB_cache})
  message(SEND_ERROR "Cache variable not defined: LIB_cache")
endif()
message(STATUS "CACHED LIB_cache='$CACHE{LIB_cache}'")
unset(LIB_cache CACHE)
message(STATUS "LIB_cache='${LIB_cache}'")


set(LIB_cache "libPrefixInPATH.a" CACHE FILEPATH "")
unset(LIB_cache)
# simulate cache variable defined in command line
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/lib/libPrefixInPATH.a" DESTINATION "${CMAKE_BINARY_DIR}")
set_property(CACHE LIB_cache PROPERTY TYPE UNINITIALIZED)
find_library(LIB_cache
  NAMES PrefixInPATH
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{LIB_cache})
  message(SEND_ERROR "Cache variable not defined: LIB_cache")
endif()
message(STATUS "CACHED LIB_cache='$CACHE{LIB_cache}'")
unset(LIB_cache CACHE)
message(STATUS "LIB_cache='${LIB_cache}'")
