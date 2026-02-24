enable_language(C)
enable_testing()

create_test_sourcelist(DRIVER_SRCS
  main.c
  case_one.c
  case_two.c
  case_three.c
)

add_executable(test_driver ${DRIVER_SRCS})

discover_tests(COMMAND test_driver
  DISCOVERY_ARGS -N
  DISCOVERY_MATCH ".*"
  TEST_NAME "\\0"
  TEST_ARGS "\\0"
)
