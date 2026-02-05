include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(shared SHARED ${CMAKE_CURRENT_LIST_DIR}/main.c)

find_package(foo PATHS ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(shared PUBLIC foo::foo)

install(
  TARGETS shared
  EXPORT shared_targets
  DESTINATION .
)
