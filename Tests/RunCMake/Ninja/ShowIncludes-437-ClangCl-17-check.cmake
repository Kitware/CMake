# 'clang-cl /showIncludes' prefix for clang-cl <= 17.
set(expect "Note: including file: ")
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
