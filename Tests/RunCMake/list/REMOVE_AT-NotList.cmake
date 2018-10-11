unset(nosuchlist)
list(REMOVE_AT nosuchlist 0)
if (DEFINED nosuchlist)
  message(FATAL_ERROR
      "list(REMOVE_AT) created our list")
endif ()
