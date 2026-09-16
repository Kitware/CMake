
enable_language(C)

if(RULE foo)
  message(SEND_ERROR "RULE 'foo' unexpectedly found")
endif()

add_custom_rule(foo OUTPUT out COMMAND cmd)
if(NOT RULE foo)
  message(SEND_ERROR "RULE 'foo' unexpectedly not found")
endif()

add_library(foo)
add_subdirectory(subdir2)
if(RULE simple)
  message(SEND_ERROR "RULE 'simple' from 'subdir2' unexpectedly found")
endif()

add_subdirectory(subdir1)
if(NOT RULE simple)
  message(SEND_ERROR "RULE 'simple' from 'subdir1' unexpectedly not found")
endif()
