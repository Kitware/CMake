# 'cl /showIncludes' prefix with 'VSLANG=1041' and 'chcp 932'.
string(ASCII 131 129 131 130 58 32 131 67 131 147 131 78 131 139 129 91 131 104 32 131 116 131 64 131 67 131 139 58 32 32 expect)
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
