set(CMAKE_REQUIRE_FIND_PACKAGE_NotHere ON)
find_package(NotHere MODULE)
message(FATAL_ERROR "This error must not be reachable.")
