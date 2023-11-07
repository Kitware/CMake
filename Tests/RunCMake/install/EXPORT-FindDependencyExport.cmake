enable_language(C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(P1 REQUIRED)
find_package(P2 REQUIRED)
find_package(P3 REQUIRED)

add_library(mylib INTERFACE)
target_link_libraries(mylib INTERFACE lib1 lib2 lib3)
install(TARGETS mylib EXPORT mylib-targets)
export(SETUP mylib-targets
  PACKAGE_DEPENDENCY P2
    ENABLED AUTO
  PACKAGE_DEPENDENCY P3
    ENABLED OFF
  )
install(EXPORT mylib-targets EXPORT_PACKAGE_DEPENDENCIES FILE mylib-targets.cmake DESTINATION lib/cmake/mylib)
