set(CMAKE_FIND_DEBUG_MODE ON)
find_package(NotHere CONFIG)
message(WARNING "This warning must be reachable.")
set(CMAKE_FIND_DEBUG_MODE OFF)
