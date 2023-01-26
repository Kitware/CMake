# 'cl /showIncludes' prefix with 'VSLANG=2052' and 'chcp 54936'.
string(ASCII 215 162 210 226 58 32 176 252 186 172 206 196 188 254 58 32 32 expect)
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
