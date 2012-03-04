get_property(FOO_BRIEF GLOBAL PROPERTY FOO BRIEF_DOCS)
get_property(FOO_FULL GLOBAL PROPERTY FOO FULL_DOCS)

if (NOT FOO_BRIEF STREQUAL "NOTFOUND")
  message(SEND_ERROR "property FOO has BRIEF_DOCS set to '${FOO_BRIEF}'")
endif ()

if (NOT FOO_FULL STREQUAL "NOTFOUND")
  message(SEND_ERROR "property FOO has FULL_DOCS set to '${FOO_FULL}'")
endif ()
