include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_executable(application ${CMAKE_CURRENT_LIST_DIR}/main.c)

find_package(
  bar 1.3.4 REQUIRED
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(application PUBLIC bar::bar)

install(
  TARGETS application
  EXPORT application_targets
  DESTINATION .
)
