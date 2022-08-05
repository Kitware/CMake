include(FetchContent)

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE BOGUS)

FetchContent_Declare(
  AddedProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/AddedProject
)
