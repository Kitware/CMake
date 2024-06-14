# 'clang-cl /showIncludes' prefix for clang-cl >= 18.
set(expect "Note: including file: ")
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
