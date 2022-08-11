# Force the provider to be invoked for each method
find_package(AThing QUIET)
message(STATUS "AThing_FOUND = ${AThing_FOUND}")

# These declared details should always succeed when used
include(FetchContent)
FetchContent_Declare(SomeDep
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
  SOURCE_SUBDIR DoesNotExist
)
FetchContent_MakeAvailable(SomeDep)
message(STATUS "FetchContent_MakeAvailable() succeeded")
