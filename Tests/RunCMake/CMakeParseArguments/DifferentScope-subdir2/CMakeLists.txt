include(CMakeParseArguments)

macro(mymacro)
  set(options )
  set(oneValueArgs)
  set(multiValueArgs)
  cmake_parse_arguments(MY_ "${options}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}" )
endmacro()

mymacro()
