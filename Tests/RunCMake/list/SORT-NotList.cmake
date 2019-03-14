unset(nosuchlist)
list(SORT nosuchlist)
if (DEFINED nosuchlist)
  message(FATAL_ERROR
    "list(SORT) created our list")
endif ()
