set(CMAKE_FIND_REQUIRED ON CACHE BOOL "") # The cache entry must be shadowed by a nested definition.
set(RequiredVarNested_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(RequiredVarNested CONFIG)
