message(AUTHOR_WARNING "Some author warning")

# without -Wno-author this will also cause an AUTHOR_WARNING message, checks
# that messages issued outside of the message command, by other CMake commands,
# also are affected by -Wno-author
include("")
