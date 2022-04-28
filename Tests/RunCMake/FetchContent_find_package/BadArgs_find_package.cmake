include(FetchContent)

FetchContent_Declare(
  AddedProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
  # The following two args are mutually exclusive
  OVERRIDE_FIND_PACKAGE
  FIND_PACKAGE_ARGS
)
