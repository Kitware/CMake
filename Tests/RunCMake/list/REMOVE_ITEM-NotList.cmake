unset(nosuchlist)
list(REMOVE_ITEM nosuchlist alpha)
if (DEFINED nosuchlist)
  message(FATAL_ERROR
    "list(REMOVE_ITEM) created our list")
endif ()
