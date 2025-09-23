set(CMAKE_FIND_REQUIRED ON)
find_package(DoesNotExist-Optional OPTIONAL CompA CompB CompC)
find_package(DoesNotExist)
message(FATAL_ERROR "This error must not be reachable.")
