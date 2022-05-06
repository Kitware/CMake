include(FetchContent)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageConfigs)

FetchContent_Declare(
  AddedProject
  # Ensure failure if we don't re-route to find_package()
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
  FIND_PACKAGE_ARGS REQUIRED
)

# Cycle through a few calls to exercise global property changes
FetchContent_MakeAvailable(AddedProject)
find_package(AddedProject REQUIRED)
FetchContent_MakeAvailable(AddedProject)  # Will re-route to find_package() again
