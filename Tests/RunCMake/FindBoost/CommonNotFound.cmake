set(CMAKE_FIND_USE_CMAKE_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
# Make sure to use the module mode signature here to not bypass FindBoost
find_package(Boost 1.80 COMPONENTS timer foobar)
