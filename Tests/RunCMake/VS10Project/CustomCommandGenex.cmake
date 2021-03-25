add_custom_command(
  OUTPUT "$<1:out.txt>"
  COMMAND ${CMAKE_COMMAND} -E touch "out.txt"
  VERBATIM
  )
add_custom_command(
  OUTPUT "out-$<CONFIG>.txt"
  COMMAND ${CMAKE_COMMAND} -E touch "out-$<CONFIG>.txt"
  VERBATIM
  )
add_custom_command(
  OUTPUT "out-$<CONFIG>-$<CONFIG>.txt"
  COMMAND ${CMAKE_COMMAND} -E touch "out-$<CONFIG>-$<CONFIG>.txt"
  VERBATIM
  )
add_custom_command(
  OUTPUT "out-$<CONFIG>-$<CONFIG:Debug>.txt"
  COMMAND ${CMAKE_COMMAND} -E touch "out-$<CONFIG>-$<CONFIG:Debug>.txt"
  VERBATIM
  )
add_custom_target(foo DEPENDS "out.txt" "out-$<CONFIG>.txt" "out-$<CONFIG>-$<CONFIG>.txt" "out-$<CONFIG>-$<CONFIG:Debug>.txt")
