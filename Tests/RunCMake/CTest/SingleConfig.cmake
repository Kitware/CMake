include(CTest)

# This should be ignored by single-config generators.
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE INTERNAL "Supported configuration types")

add_test(NAME SingleConfig COMMAND ${CMAKE_COMMAND} -E echo SingleConfig)
