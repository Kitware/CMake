# 'cl /showIncludes' prefix with 'VSLANG=1036' and 'chcp 437'.
string(ASCII 82 101 109 97 114 113 117 101 255 58 32 105 110 99 108 117 115 105 111 110 32 100 117 32 102 105 99 104 105 101 114 255 58 32 32 expect)
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
