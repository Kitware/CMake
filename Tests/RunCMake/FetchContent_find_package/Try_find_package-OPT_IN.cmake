include(FetchContent)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageConfigs)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE OPT_IN)

# With opt-in, should call find_package()
FetchContent_Declare(
    FirstProject
    # Ensure failure if we don't re-route to find_package()
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
    FIND_PACKAGE_ARGS REQUIRED
)

# Without opt-in, shouldn't call find_package()
FetchContent_Declare(
    SecondProject
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
)

FetchContent_MakeAvailable(FirstProject SecondProject)
