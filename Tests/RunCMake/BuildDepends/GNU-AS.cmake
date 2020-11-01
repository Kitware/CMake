enable_language(ASM)

# Validate undocumented implementation detail.
message(STATUS "CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_MATCH='${CMAKE_ASM${ASM_DIALECT}_COMPILER_ID_VENDOR_MATCH}'")

add_library(gnu_as STATIC gnu_as.s)
target_include_directories(gnu_as PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(gnu_as PRIVATE "TEST_DEF=Hello")

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(check_pairs
  \"$<TARGET_FILE:gnu_as>|${CMAKE_CURRENT_BINARY_DIR}/gnu_as.inc\"
  )
")
