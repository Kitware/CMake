cmake_policy(SET CMP0070 NEW)

set(text)
string(APPEND text "$<$<CONFIG:CustomConfig>:1>")
string(APPEND text "$<$<CONFIG:Release>:2>")
string(APPEND text "$<$<CONFIG:Debug,Release>:3>")
string(APPEND text "$<$<CONFIG:Release,CustomConfig,Debug>:4>")
file(GENERATE OUTPUT CONFIG-multiple-entries-generated.txt CONTENT "${text}")
