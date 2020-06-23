cmake_policy(SET CMP0070 NEW)

set(text)
string(APPEND text "$<$<CONFIG:>:1>")
string(APPEND text "$<$<CONFIG:Release,>:2>")
string(APPEND text "$<$<CONFIG:,Release>:3>")
string(APPEND text "$<$<CONFIG:Release,,Debug>:4>")
string(APPEND text "$<$<CONFIG:Release,Debug>:5>")
file(GENERATE OUTPUT CONFIG-empty-entries-generated.txt CONTENT  ${text})
