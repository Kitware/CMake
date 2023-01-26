# 'cl /showIncludes' prefix with 'VSLANG=2052' and 'chcp 65001'.
string(ASCII 230 179 168 230 132 143 58 32 229 140 133 229 144 171 230 150 135 228 187 182 58 32 32 expect)
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
