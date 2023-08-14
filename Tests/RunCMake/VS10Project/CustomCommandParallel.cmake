cmake_policy(VERSION 3.26) # CMP0147 left unset
add_custom_command(OUTPUT "cmp0147-old.txt" COMMAND echo)
cmake_policy(SET CMP0147 NEW)
add_custom_command(OUTPUT "cmp0147-new.txt" COMMAND echo)
add_custom_target(foo DEPENDS "cmp0147-old.txt" "cmp0147-new.txt")
