include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)
include(GNUInstallDirs)

add_library(liba INTERFACE)

install(TARGETS liba EXPORT setA DESTINATION .)
