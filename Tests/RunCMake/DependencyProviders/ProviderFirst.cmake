include(FetchContent)
FetchContent_Declare(SomeDep
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
  SOURCE_SUBDIR DoesNotExist
  FIND_PACKAGE_ARGS NO_DEFAULT_PATH COMPONENTS abc
)
FetchContent_MakeAvailable(SomeDep)
message(STATUS "FetchContent_MakeAvailable() succeeded")
