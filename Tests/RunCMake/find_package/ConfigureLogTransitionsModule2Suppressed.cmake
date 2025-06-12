set(CMAKE_FIND_DEBUG_MODE_NO_IMPLICIT_CONFIGURE_LOG 1)

# Stable sorting for predictable behaviors.
set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)

# Unset search variables for more predictable output.
unset(CMAKE_FRAMEWORK_PATH)
unset(CMAKE_APPBUNDLE_PATH)
unset(ENV{CMAKE_PREFIX_PATH})
unset(ENV{CMAKE_FRAMEWORK_PATH})
unset(ENV{CMAKE_APPBUNDLE_PATH})

message("NotDefined -> NotFound")
message(CONFIGURE_LOG "NotDefined -> NotFound")
find_package(ViaModule)

message("NotFound -> NotFound")
message(CONFIGURE_LOG "NotFound -> NotFound")
find_package(ViaModule)

list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_LIST_DIR}/ConfigureLog/cmake")
list(INSERT CMAKE_PREFIX_PATH 0
  "${CMAKE_CURRENT_LIST_DIR}/ConfigureLog")

message("NotFound -> Found")
message(CONFIGURE_LOG "NotFound -> Found")
find_package(ViaModule)

message("Found -> Found")
message(CONFIGURE_LOG "Found -> Found")
find_package(ViaModule)

message("Found -> NotFound")
message(CONFIGURE_LOG "Found -> NotFound")
list(REMOVE_AT CMAKE_PREFIX_PATH 0)
list(REMOVE_AT CMAKE_MODULE_PATH 0)

find_package(ViaModule)

message("END")
message(CONFIGURE_LOG "END")
