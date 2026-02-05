include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(mammal INTERFACE)
add_library(canine INTERFACE)
target_link_libraries(canine INTERFACE mammal)

install(TARGETS canine EXPORT dog DESTINATION .)
