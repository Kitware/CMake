set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES "1942b4fa-b2c5-4546-9385-83f254070067")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

enable_language(CXX)

find_package(P1)
find_package(P2)
find_package(P3)
find_package(P4)

add_library(mylib STATIC empty.cpp)
target_link_libraries(mylib PRIVATE l1 l2 l3 l4)

install(TARGETS mylib EXPORT mytargets)
export(EXPORT mytargets EXPORT_PACKAGE_DEPENDENCIES FILE mytargets.cmake)
