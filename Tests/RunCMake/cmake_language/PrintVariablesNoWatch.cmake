# Enumerating ALL reads values via the snapshot API, not makefile.GetDefinition,
# so it must not fire variable_watch callbacks. If the watch fires during the
# walk, the test fails.
function(watch_fired)
  message(FATAL_ERROR "variable_watch fired during PRINT_VARIABLES enumeration")
endfunction()
set(watched "value")
variable_watch(watched watch_fired)
cmake_language(PRINT_VARIABLES ALL NAME_REGEX "^watched$")
