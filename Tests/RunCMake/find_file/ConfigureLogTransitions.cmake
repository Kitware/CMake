set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

message(CONFIGURE_LOG "NotDefined -> NotFound")
find_file(NOEXIST_FILE NAMES NoExist.h)

message(CONFIGURE_LOG "NotFound -> NotFound")
find_file(NOEXIST_FILE NAMES NoExist.h)

message(CONFIGURE_LOG "NotDefined -> Found")
find_file(PREFIX_IN_PATH NAMES PrefixInPATH.h)

message(CONFIGURE_LOG "Found -> Found")
find_file(PREFIX_IN_PATH NAMES PrefixInPATH.h)

message(CONFIGURE_LOG "Found -> NotFound")
unset(PREFIX_IN_PATH CACHE)
unset(CMAKE_PREFIX_PATH)
find_file(PREFIX_IN_PATH NAMES PrefixInPATH.h)

message(CONFIGURE_LOG "NotFound -> Found")
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_file(PREFIX_IN_PATH NAMES PrefixInPATH.h)
