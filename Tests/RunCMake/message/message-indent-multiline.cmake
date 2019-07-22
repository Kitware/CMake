# NOTE Use non-space indent string, to check indentation
# of line endings and "empty" lines.
# ALERT Do not put any space characters after the non-space!
list(APPEND CMAKE_MESSAGE_INDENT " >")
set(msg [[This is
the multiline
message]]) # No `\n` at the end!
# NOTE Two empty lines after the text
message(STATUS "${msg}\n\n")
message(STATUS "${msg}")
# This is just to make sure NOTICE messages are also get indented:
# https://gitlab.kitware.com/cmake/cmake/issues/19418#note_588011
message(NOTICE "${msg}")
