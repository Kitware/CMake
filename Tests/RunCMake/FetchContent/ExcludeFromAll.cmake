enable_language(C)

include(FetchContent)

FetchContent_Declare(
  ExcludeFromAll
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/ExcludeFromAll
  EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(ExcludeFromAll)
