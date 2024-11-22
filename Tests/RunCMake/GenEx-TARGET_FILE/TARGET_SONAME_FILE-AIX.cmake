enable_language (C)
set(CMAKE_AIX_SHARED_LIBRARY_ARCHIVE 1)
add_library(sla SHARED empty.c)
add_custom_target(custom COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_SONAME_FILE:sla>")
