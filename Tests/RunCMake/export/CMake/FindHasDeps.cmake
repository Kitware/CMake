find_package(Threads REQUIRED)
find_package(P4 REQUIRED)

add_library(HasDeps::interface IMPORTED INTERFACE)
target_link_libraries(HasDeps::interface INTERFACE Threads::Threads l4)

add_library(HasDeps::A IMPORTED UNKNOWN)
target_link_libraries(HasDeps::A INTERFACE HasDeps::interface)
file(TOUCH "${CMAKE_CURRENT_BINARY_DIR}/a.so")
set_property(TARGET HasDeps::A PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/a.so")

add_library(HasDeps::B IMPORTED UNKNOWN)
target_link_libraries(HasDeps::B INTERFACE HasDeps::interface)
file(TOUCH "${CMAKE_CURRENT_BINARY_DIR}/b.so")
set_property(TARGET HasDeps::B PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/b.so")

set(HASDEPS_FOUND TRUE)
