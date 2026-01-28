
add_library(foo INTERFACE)

target_sources(foo INTERFACE FILE_SET foo1 TYPE HEADERS)
target_sources(foo INTERFACE FILE_SET foo2 TYPE HEADERS)

# check various supported syntax
set_property(FILE_SET TARGET foo PROPERTY FOO)
set_property(FILE_SET TARGET foo PROPERTY FOO BAR)
set_property(FILE_SET TARGET foo PROPERTY FOO BAR BAR2)

set_property(FILE_SET foo1 TARGET foo PROPERTY FOO)
set_property(FILE_SET foo1 TARGET foo PROPERTY FOO BAR)
set_property(FILE_SET foo1 TARGET foo PROPERTY FOO BAR BAR2)
get_property(value FILE_SET foo1 TARGET foo PROPERTY FOO)
if(NOT value STREQUAL "BAR;BAR2")
  message(SEND_ERROR "FILE_SET foo1 TARGET foo: \"${value}\", expected \"BAR;BAR2\"")
endif()

set_property(FILE_SET foo1 foo2 TARGET foo PROPERTY FOO)
set_property(FILE_SET foo1 foo2 TARGET foo PROPERTY FOO BAR)
set_property(FILE_SET foo1 foo2 TARGET foo PROPERTY FOO BAR BAR2)
get_property(value FILE_SET foo1 TARGET foo PROPERTY FOO)
if(NOT value STREQUAL "BAR;BAR2")
  message(SEND_ERROR "FILE_SET foo1 TARGET foo: \"${value}\", expected \"BAR;BAR2\"")
endif()
get_property(value FILE_SET foo2 TARGET foo PROPERTY FOO)
if(NOT value STREQUAL "BAR;BAR2")
  message(SEND_ERROR "FILE_SET foo2 TARGET foo: \"${value}\", expected \"BAR;BAR2\"")
endif()
