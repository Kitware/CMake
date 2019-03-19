include(FetchContent)

FetchContent_Declare(
  WithProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithProject
)

message(STATUS "Before first")
FetchContent_MakeAvailable(WithProject)
message(STATUS "Between both")
FetchContent_MakeAvailable(WithProject)
message(STATUS "After last")
