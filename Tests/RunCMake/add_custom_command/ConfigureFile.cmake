configure_file(ConfigureFile.in foo.txt @ONLY)
add_custom_target(foo)
add_custom_command(
  OUTPUT bar.txt
  MAIN_DEPENDENCY ConfigureFile.in # Attach to input of configure_file
  DEPENDS foo
  COMMAND ${CMAKE_COMMAND} -E copy foo.txt bar.txt
  )
add_custom_target(bar DEPENDS bar.txt)
