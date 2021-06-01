find_program(PROG_A
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (DEFINED CACHE{PROG_A})
  message(SEND_ERROR "Cache variable defined: PROG_A")
endif()
message(STATUS "PROG_A='${PROG_A}'")


find_program(PROG_AandB
  NAMES testAandB
  NO_CACHE
  )
if (DEFINED CACHE{PROG_AandB})
  message(SEND_ERROR "Cache variable defined: PROG_AandN")
endif()
message(STATUS "PROG_AandB='${PROG_AandB}'")


cmake_policy(SET CMP0125 OLD)
message(STATUS "")
message(STATUS "Policy CMP0125 = OLD")
file(REMOVE "${CMAKE_BINARY_DIR}/testA")

set(PROG_cache "unknown" CACHE FILEPATH "")
find_program(PROG_cache
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PROG_cache})
  message(SEND_ERROR "Cache variable not defined: PROG_cache")
endif()
message(STATUS "CACHED PROG_cache='$CACHE{PROG_cache}'")
unset(PROG_cache CACHE)
message(STATUS "PROG_cache='${PROG_cache}'")


set(PROG_cache "testA" CACHE FILEPATH "")
unset(PROG_cache)
find_program(PROG_cache
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PROG_cache})
  message(SEND_ERROR "Cache variable not defined: PROG_cache")
endif()
message(STATUS "CACHED PROG_cache='$CACHE{PROG_cache}'")
unset(PROG_cache CACHE)
message(STATUS "PROG_cache='${PROG_cache}'")


set(PROG_cache "testA" CACHE FILEPATH "")
unset(PROG_cache)
# simulate cache variable defined in command line
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/A/testA" DESTINATION "${CMAKE_BINARY_DIR}")
set_property(CACHE PROG_cache PROPERTY TYPE UNINITIALIZED)
find_program(PROG_cache
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PROG_cache})
  message(SEND_ERROR "Cache variable not defined: PROG_cache")
endif()
message(STATUS "CACHED PROG_cache='$CACHE{PROG_cache}'")
unset(PROG_cache CACHE)
message(STATUS "PROG_cache='${PROG_cache}'")


cmake_policy(SET CMP0125 NEW)
message(STATUS "")
message(STATUS "Policy CMP0125 = NEW")
file(REMOVE "${CMAKE_BINARY_DIR}/testA")

set(PROG_cache "unknown" CACHE FILEPATH "")
find_program(PROG_cache
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PROG_cache})
  message(SEND_ERROR "Cache variable not defined: PROG_cache")
endif()
message(STATUS "CACHED PROG_cache='$CACHE{PROG_cache}'")
unset(PROG_cache CACHE)
message(STATUS "PROG_cache='${PROG_cache}'")


set(PROG_cache "testA" CACHE FILEPATH "")
unset(PROG_cache)
find_program(PROG_cache
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PROG_cache})
  message(SEND_ERROR "Cache variable not defined: PROG_cache")
endif()
message(STATUS "CACHED PROG_cache='$CACHE{PROG_cache}'")
unset(PROG_cache CACHE)
message(STATUS "PROG_cache='${PROG_cache}'")


set(PROG_cache "testA" CACHE FILEPATH "")
unset(PROG_cache)
# simulate cache variable defined in command line
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/A/testA" DESTINATION "${CMAKE_BINARY_DIR}")
set_property(CACHE PROG_cache PROPERTY TYPE UNINITIALIZED)
find_program(PROG_cache
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_CACHE
  NO_DEFAULT_PATH
  )
if (NOT DEFINED CACHE{PROG_cache})
  message(SEND_ERROR "Cache variable not defined: PROG_cache")
endif()
message(STATUS "CACHED PROG_cache='$CACHE{PROG_cache}'")
unset(PROG_cache CACHE)
message(STATUS "PROG_cache='${PROG_cache}'")
