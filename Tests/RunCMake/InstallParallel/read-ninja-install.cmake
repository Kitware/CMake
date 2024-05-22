set(ninja_log ${CMAKE_ARGV3})
file(STRINGS ${ninja_log} lines)
list(POP_FRONT lines)
list(FILTER lines INCLUDE REGEX ".*install.*util")
