add_custom_command(OUTPUT out.txt COMMAND false)
add_custom_target(drive1 DEPENDS out.txt)
add_custom_target(drive2 DEPENDS out.txt)
