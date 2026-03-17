message(AUTHOR_WARNING "Some author warning")

# with -Wauthor this will also cause an AUTHOR_WARNING message, checks that
# messages issued outside of the message command, by other CMake commands, also
# are affected by -Wauthor
include("")
