include(FetchContent)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageConfigs)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)

FetchContent_Declare(
  AddedProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
  FIND_PACKAGE_ARGS REQUIRED
)

FetchContent_MakeAvailable(AddedProject)
