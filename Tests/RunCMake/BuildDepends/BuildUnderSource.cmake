enable_language(C)
include_directories(include)
add_executable(BuildUnderSource BuildUnderSource.c)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(check_pairs
  \"$<TARGET_FILE:BuildUnderSource>|${CMAKE_CURRENT_SOURCE_DIR}/include/BuildUnderSource.h\"
  )
")
