include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_executable(test
  ${CMAKE_CURRENT_LIST_DIR}/main.c
)

find_package(
  baz REQUIRED
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(test PUBLIC baz)

install(
  TARGETS test
  EXPORT test_targets
)
