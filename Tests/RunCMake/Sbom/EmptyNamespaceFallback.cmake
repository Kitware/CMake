include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)
include(GNUInstallDirs)

add_library(liba INTERFACE)
add_library(libb INTERFACE)
target_link_libraries(libb INTERFACE liba)

install(TARGETS liba EXPORT setA DESTINATION .)
install(TARGETS libb EXPORT setB DESTINATION .)
