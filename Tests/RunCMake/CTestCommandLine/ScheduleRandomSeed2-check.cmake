string(REGEX MATCHALL "Start [1-5]" ScheduleRandomSeed2_ORDER "${actual_stdout}")
get_property(ScheduleRandomSeed1_ORDER DIRECTORY PROPERTY ScheduleRandomSeed1_ORDER)
if(NOT "${ScheduleRandomSeed1_ORDER}" STREQUAL "${ScheduleRandomSeed2_ORDER}")
  string(CONCAT RunCMake_TEST_FAILED
    "ScheduleRandomSeed1 order:\n"
    " ${ScheduleRandomSeed1_ORDER}\n"
    "does not match ScheduleRandomSeed2 order:\n"
    " ${ScheduleRandomSeed2_ORDER}\n"
    )
endif()
