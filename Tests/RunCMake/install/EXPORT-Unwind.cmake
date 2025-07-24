set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES "1942b4fa-b2c5-4546-9385-83f254070067")
enable_language(C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(P1 REQUIRED)

add_library(Unwind INTERFACE)
target_link_libraries(Unwind INTERFACE lib1)
install(TARGETS Unwind EXPORT UnwindTargets)
install(EXPORT UnwindTargets EXPORT_PACKAGE_DEPENDENCIES DESTINATION lib/cmake/Unwind)
install(FILES cmake/UnwindConfig.cmake DESTINATION lib/cmake/Unwind)
