enable_language(C)

add_custom_command(
  OUTPUT main.c
  COMMAND ${CMAKE_COMMAND} -E copy main.c.in main.c
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/main.c.in
  )
add_executable(main main.c)
set_property(TARGET main PROPERTY UNITY_BUILD ON)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(check_pairs
  \"$<TARGET_FILE:main>|${CMAKE_CURRENT_BINARY_DIR}/main.c.in\"
  \"$<TARGET_FILE:main>|${CMAKE_CURRENT_BINARY_DIR}/main.c\"
  )
set(check_exes
  \"$<TARGET_FILE:main>\"
  )
")
