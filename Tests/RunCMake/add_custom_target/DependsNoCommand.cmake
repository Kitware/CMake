# Custom targets with DEPENDS but no COMMAND, on a file and on the output of
# a custom command without a COMMAND.
add_custom_target(dependsOnFile ALL DEPENDS "${CMAKE_CURRENT_LIST_FILE}")

add_custom_command(OUTPUT noCommand.txt DEPENDS "${CMAKE_CURRENT_LIST_FILE}")
set_property(SOURCE noCommand.txt PROPERTY SYMBOLIC 1)
add_custom_target(dependsOnNoCommand ALL DEPENDS noCommand.txt)
