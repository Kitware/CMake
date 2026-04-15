# This should still produce a warning when -Wno-error=author is specified
message(AUTHOR_WARNING "Some author warning")

# with -Wno-error=author this will also cause an AUTHOR_WARNING message, checks
# that messages issued outside of the message command, by other CMake commands,
# also are affected by -Wno-error=author
include("")
