include(FetchContent)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageConfigs)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)

FetchContent_Declare(
  FirstProject
  # Ensure failure if we don't re-route to find_package()
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
)

FetchContent_Declare(
    SecondProject
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
    OVERRIDE_FIND_PACKAGE  # Takes precedence over ALWAYS mode
)

FetchContent_MakeAvailable(FirstProject SecondProject)
