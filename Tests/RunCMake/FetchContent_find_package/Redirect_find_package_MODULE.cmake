include(FetchContent)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageConfigs)

FetchContent_Declare(
    FirstProject
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
    OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  SecondProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
  # Allow a call to find_package() that we know will fail.
  # This enables redirection of calls to find_package(SecondProject)
  # after FetchContent_MakeAvailable() populates.
  FIND_PACKAGE_ARGS NAMES I_do_not_exist
)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageFindModules)

# Re-directs to FetchContent_MakeAvailable()
message(STATUS "find_package(FirstProject):")
find_package(FirstProject REQUIRED MODULE)
message(STATUS "FirstProject_FOUND = ${FirstProject_FOUND}")

# Does nothing, already populated
message(STATUS "FetchContent_MakeAvailable(FirstProject):")
FetchContent_MakeAvailable(FirstProject)

# Populates as normal
message(STATUS "FetchContent_MakeAvailable(SecondProject):")
FetchContent_MakeAvailable(SecondProject)

# Redirects to config package file created by previous command
message(STATUS "find_package(SecondProject):")
find_package(SecondProject REQUIRED MODULE)
message(STATUS "SecondProject_FOUND = ${FirstProject_FOUND}")

message(STATUS "End of test")
