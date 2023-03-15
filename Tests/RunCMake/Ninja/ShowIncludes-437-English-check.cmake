# 'cl /showIncludes' prefix with 'VSLANG=1033' and 'chcp 437'.
set(expect "Note: including file: ")
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
