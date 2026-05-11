enable_language(C)

# A custom command produces a header that does not exist at configure time.
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated.h"
  COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/generated.h"
)

# An INTERFACE library generates the header.
add_library(header_provider INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/generated.h)
target_include_directories(header_provider INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")
# Not in ALL so it is only built when something depends on it.
set_property(TARGET header_provider PROPERTY EXCLUDE_FROM_ALL 1)

# A consumer uses $<COMPILE_ONLY:header_provider> so it gets the include
# path but does not link against header_provider.  The build-order
# dependency on header_provider must be propagated through the
# COMPILE_ONLY expression so that generated.h exists when consumer.c
# is compiled.
add_library(consumer SHARED COMPILE_ONLY-custom-target-deps-consumer.c)
target_link_libraries(consumer PRIVATE "$<COMPILE_ONLY:header_provider>")
