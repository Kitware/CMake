add_custom_command(
  OUTPUT warn.txt
  COMMAND ${CMAKE_COMMAND} -E touch warn.txt
  DEPFILE warn.d
  )
cmake_policy(SET CMP0116 OLD)
add_custom_command(
  OUTPUT old.txt
  COMMAND ${CMAKE_COMMAND} -E touch old.txt
  DEPFILE old.d
  )
cmake_policy(SET CMP0116 NEW)
add_custom_command(
  OUTPUT new.txt
  COMMAND ${CMAKE_COMMAND} -E touch new.txt
  DEPFILE new.d
  )
add_custom_target(cc ALL DEPENDS warn.txt old.txt new.txt)
