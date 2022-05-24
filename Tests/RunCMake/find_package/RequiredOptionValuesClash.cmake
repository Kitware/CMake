set(CMAKE_DISABLE_FIND_PACKAGE_Foo ON)
set(CMAKE_REQUIRE_FIND_PACKAGE_Foo ON)

find_package(Foo)
message(FATAL_ERROR "This error must not be reachable.")
