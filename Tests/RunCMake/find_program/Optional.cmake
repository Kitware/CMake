set(CMAKE_FIND_REQUIRED ON)
find_program(PROG_AandB_Optional
  NAMES testAandB
  OPTIONAL
)
find_program(PROG_AandB_OptionalRequired
  NAMES testAandB
  OPTIONAL
  REQUIRED
)
find_program(PROG_AandB
  NAMES testAandB
)
