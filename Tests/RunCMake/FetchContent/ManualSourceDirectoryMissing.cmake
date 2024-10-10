message(STATUS "FETCHCONTENT_SOURCE_DIR_WITHPROJECT = ${FETCHCONTENT_SOURCE_DIR_WITHPROJECT}")
include(FetchContent)

FetchContent_Declare(
  WithProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithProject
)

FetchContent_MakeAvailable(WithProject)
