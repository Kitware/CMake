set(nosuchlist "")
list(REMOVE_AT nosuchlist 0)
if (NOT DEFINED nosuchlist OR NOT nosuchlist STREQUAL "")
  message(FATAL_ERROR
    "list(REMOVE_AT) modified our list")
endif ()
