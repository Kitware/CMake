set(original "/original/value/long/enough/to/live/on/the/heap")
set(replacement "/replacement/value/long/enough/to/live/on/the/heap")

# The first watch replaces the variable on the access type in 'armed'.
# The second watch must still get the value of that access.
macro(replace var access value)
  if("${access}" STREQUAL "${armed}" AND NOT "${value}" STREQUAL "${replacement}")
    set(${var} "${replacement}")
  endif()
endmacro()
function(check var access value)
  if(access STREQUAL armed)
    message(STATUS "${var} ${access} ${value}")
  endif()
endfunction()

variable_watch(r replace)
variable_watch(r check)
set(r "${original}")
set(armed READ_ACCESS)
set(tmp "${r}")
unset(armed)

variable_watch(p replace)
variable_watch(p check)
block(PROPAGATE p)
  set(p "${original}")
  set(armed MODIFIED_ACCESS)
endblock()
