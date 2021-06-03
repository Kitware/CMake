find_path(PATH_exists
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (DEFINED CACHE{PATH_exists})
  message(SEND_ERROR "Cache variable defined: PATH_exists")
endif()
message(STATUS "PATH_exists='${PATH_exists}'")


find_path(PATH_doNotExists
  NAMES doNotExists.h
  NO_CACHE
  )
if (DEFINED CACHE{PATH_doNotExists})
  message(SEND_ERROR "Cache variable defined: PATH_doNotExists")
endif()
message(STATUS "PATH_exists='${PATH_doNotExists}'")


cmake_policy(SET CMP0125 OLD)
message(STATUS "")
message(STATUS "Policy CMP0125 = OLD")
file(REMOVE "${CMAKE_BINARY_DIR}/PrefixInPATH.h")

set(PATH_cache "unknown" CACHE PATH "")
find_path(PATH_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PATH_cache})
  message(SEND_ERROR "Cache variable defined: PATH_cache")
endif()
message(STATUS "CACHED PATH_cache='$CACHE{PATH_cache}'")
unset(PATH_cache CACHE)
message(STATUS "PATH_cache='${PATH_cache}'")


set(PATH_cache "include" CACHE PATH "")
unset(PATH_cache)
find_path(PATH_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PATH_cache})
  message(SEND_ERROR "Cache variable defined: PATH_cache")
endif()
message(STATUS "CACHED PATH_cache='$CACHE{PATH_cache}'")
unset(PATH_cache CACHE)
message(STATUS "PATH_cache='${PATH_cache}'")


set(PATH_cache "include" CACHE PATH "")
unset(PATH_cache)
# simulate cache variable defined in command line
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/include/PrefixInPATH.h" DESTINATION "${CMAKE_BINARY_DIR}/include")
set_property(CACHE PATH_cache PROPERTY TYPE UNINITIALIZED)
find_path(PATH_cache
  NAMES PrefixInPATH.h
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PATH_cache})
  message(SEND_ERROR "Cache variable not defined: PATH_cache")
endif()
message(STATUS "CACHED PATH_cache='$CACHE{PATH_cache}'")
unset(PATH_cache CACHE)
message(STATUS "PATH_cache='${PATH_cache}'")


cmake_policy(SET CMP0125 NEW)
message(STATUS "")
message(STATUS "Policy CMP0125 = NEW")
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/include")

set(PATH_cache "unknown" CACHE PATH "")
unset(PATH_cache)
find_path(PATH_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PATH_cache})
  message(SEND_ERROR "Cache variable defined: PATH_cache")
endif()
message(STATUS "CACHED PATH_cache='$CACHE{PATH_cache}'")
unset(PATH_cache CACHE)
message(STATUS "PATH_cache='${PATH_cache}'")


set(PATH_cache "include" CACHE PATH "")
unset(PATH_cache)
find_path(PATH_cache
  NAMES PrefixInPATH.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PATH_cache})
  message(SEND_ERROR "Cache variable defined: PATH_cache")
endif()
message(STATUS "CACHED PATH_cache='$CACHE{PATH_cache}'")
unset(PATH_cache CACHE)
message(STATUS "PATH_cache='${PATH_cache}'")


set(PATH_cache "include" CACHE PATH "")
unset(PATH_cache)
# simulate cache variable defined in command line
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/include/PrefixInPATH.h" DESTINATION "${CMAKE_BINARY_DIR}/include")
set_property(CACHE PATH_cache PROPERTY TYPE UNINITIALIZED)
find_path(PATH_cache
  NAMES PrefixInPATH.h
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PATH_cache})
  message(SEND_ERROR "Cache variable not defined: PATH_cache")
endif()
message(STATUS "CACHED PATH_cache='$CACHE{PATH_cache}'")
unset(PATH_cache CACHE)
message(STATUS "PATH_cache='${PATH_cache}'")
