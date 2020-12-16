add_custom_command(OUTPUT "a" COMMAND "$<1:$<OUTPUT_CONFIG:a>>")
add_custom_command(OUTPUT "b" COMMAND "$<1:$<COMMAND_CONFIG:b>>")
add_custom_target(drive DEPENDS "a" "b")
