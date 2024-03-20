include(apple-common.cmake)

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
find_package(bad-platform-capture CONFIG REQUIRED)
find_package(bad-arch-capture CONFIG REQUIRED)

# The above packages capture their own error messages.
# In real packages they would then set _FOUND to false.
# For testing here, just print the messages.
message(STATUS "${bad-platform-capture_unsupported}")
message(STATUS "${bad-arch-capture_unsupported}")
