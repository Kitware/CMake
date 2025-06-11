set(CMAKE_FIND_DEBUG_MODE_NO_IMPLICIT_CONFIGURE_LOG 1)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

message(CONFIGURE_LOG "NotDefined -> NotFound")
find_path(NOEXIST_FILE NAMES NoExist.h)

message(CONFIGURE_LOG "NotFound -> NotFound")
find_path(NOEXIST_FILE NAMES NoExist.h)

message(CONFIGURE_LOG "NotDefined -> Found")
find_path(PREFIX_IN_PATH NAMES PrefixInPATH.h)

message(CONFIGURE_LOG "Found -> Found")
find_path(PREFIX_IN_PATH NAMES PrefixInPATH.h)

message(CONFIGURE_LOG "Found -> NotFound")
unset(PREFIX_IN_PATH CACHE)
unset(CMAKE_PREFIX_PATH)
find_path(PREFIX_IN_PATH NAMES PrefixInPATH.h)

message(CONFIGURE_LOG "NotFound -> Found")
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_path(PREFIX_IN_PATH NAMES PrefixInPATH.h)
