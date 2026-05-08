# Test warning when setting deprecated variables.
set(CMAKE_WARN_DEPRECATED OFF)
message(DEPRECATION "Test")
set(CMAKE_ERROR_DEPRECATED ON)

# Test warning when reading deprecated variables.
set(warn "${CMAKE_WARN_DEPRECATED}")
set(error "${CMAKE_ERROR_DEPRECATED}")
