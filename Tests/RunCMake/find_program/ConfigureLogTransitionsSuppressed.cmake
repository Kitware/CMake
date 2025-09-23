set(CMAKE_FIND_DEBUG_MODE_NO_IMPLICIT_CONFIGURE_LOG 1)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Prefix")

message(CONFIGURE_LOG "NotDefined -> NotFound")
find_program(NOEXIST_FILE NAMES NoExist)

message(CONFIGURE_LOG "NotFound -> NotFound")
find_program(NOEXIST_FILE NAMES NoExist)

message(CONFIGURE_LOG "NotDefined -> Found")
find_program(PREFIX_IN_PATH NAMES prog)

message(CONFIGURE_LOG "Found -> Found")
find_program(PREFIX_IN_PATH NAMES prog)

message(CONFIGURE_LOG "Found -> NotFound")
unset(PREFIX_IN_PATH CACHE)
unset(CMAKE_PREFIX_PATH)
find_program(PREFIX_IN_PATH NAMES prog)

message(CONFIGURE_LOG "NotFound -> Found")
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_program(PREFIX_IN_PATH NAMES prog)
