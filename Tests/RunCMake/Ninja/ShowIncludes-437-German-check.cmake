# 'cl /showIncludes' prefix with 'VSLANG=1031' and 'chcp 437'.
set(expect "Hinweis: Einlesen der Datei: ")
include(${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-check.cmake)
