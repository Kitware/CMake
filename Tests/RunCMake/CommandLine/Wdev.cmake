message(AUTHOR_WARNING "Some Author Warning")

# with -Wdev this will also cause an AUTHOR_WARNING message, checks that
# messages issued outside of the message command, by other CMake commands, also
# are affected by -Wdev
include("")
