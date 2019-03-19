unset(nosuchlist)
list(REVERSE nosuchlist)
if (DEFINED nosuchlist)
  message(FATAL_ERROR
    "list(REVERSE) created our list")
endif ()
