
set(ENV_PATH "$ENV{PATH}")
set(ENV{PATH} ${CMAKE_CURRENT_SOURCE_DIR}/A)
find_program(PROG
  NAMES testAandB
  )
message(STATUS "PROG='${PROG}'")
unset(PROG CACHE)

set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
find_program(PROG
  NAMES testAandB
  )
message(STATUS "PROG='${PROG}'")
unset(PROG CACHE)

find_program(PROG_A
  NAMES testAandB
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/B ${CMAKE_CURRENT_SOURCE_DIR}/A
  )
message(STATUS "PROG='${PROG_A}'")
unset(PROG_A CACHE)
set(ENV{PATH} "${ENV_PATH}")

find_program(PROG_A
  NAMES testAandB
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/A ${CMAKE_CURRENT_SOURCE_DIR}/B
  )
message(STATUS "PROG='${PROG_A}'")
unset(PROG_A CACHE)
set(ENV{PATH} "${ENV_PATH}")
