include("${CMAKE_CURRENT_BINARY_DIR}/data.cmake")

include(ExternalProject)
ExternalProject_add(external
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/ExternalProject"
  CMAKE_CACHE_ARGS
    ${cache_args}
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

set(cache_args_path "<TMP_DIR>/external-cache-$<CONFIG>.cmake")
set(cmake_cache_path "<BINARY_DIR>/CMakeCache.txt")
_ep_replace_location_tags(external cache_args_path cmake_cache_path)

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake" CONTENT "
set(check_pairs
  \"${cmake_cache_path}|${cache_args_path}\"
)
")
