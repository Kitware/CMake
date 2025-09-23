# Various newline possibilities

set(help_strings
  "\n"
  "\n\n\n"
  "\n \n"
  "\nline1"
  "line1\n"
  "\nline1\n"
  "line1\nline2\nline3"
  "\nline1\nline2\nline3\n"
  "
line line line line line line line line line line line line line line line line
line line line line line line line line line line line line line line line line
line line line line line line line line line line line line line line line line"
)

foreach(help IN LISTS help_strings)
  string(SHA1 name "${help}")
  set("${name}" "" CACHE STRING "${help}" FORCE)
endforeach()
