find_program(PROG_A
  NAMES testA
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/A
  NO_DEFAULT_PATH
  REQUIRED
  )
message(STATUS "PROG_A='${PROG_A}'")

find_program(PROG_AandB
  NAMES testAandB
  REQUIRED
  )
