include(FetchContent)

FetchContent_Declare(
  WithProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/ADirThatDoesNotExist
)

FetchContent_MakeAvailable(WithProject)
