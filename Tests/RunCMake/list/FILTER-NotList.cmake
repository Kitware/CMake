unset(nosuchlist)
list(FILTER nosuchlist EXCLUDE REGEX "^FILTER_THIS_.+")
if (DEFINED nosuchlist)
  message(FATAL_ERROR
    "list(FILTER) created our list")
endif ()
