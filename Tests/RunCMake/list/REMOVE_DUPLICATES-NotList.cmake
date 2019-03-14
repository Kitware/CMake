unset(nosuchlist)
list(REMOVE_DUPLICATES nosuchlist)
if (DEFINED nosuchlist)
  message(FATAL_ERROR
    "list(REMOVE_DUPLICATES) created our list")
endif ()
