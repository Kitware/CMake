# 'cl /showIncludes' prefix with 'VSLANG=1040' and 'chcp 437'.
set(expect "Nota: file incluso  ")
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
