include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)
include(GNUInstallDirs)

add_library(liba INTERFACE)

install(TARGETS liba EXPORT setA1 DESTINATION .)
install(TARGETS liba EXPORT setA2 DESTINATION .)
