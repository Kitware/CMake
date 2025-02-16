project(TestLibrary C)

set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/../install")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../install")

find_package(libb REQUIRED COMPONENTS libb)

add_executable(app app.c)

target_link_libraries(app PUBLIC libb::libb)

install(TARGETS app DESTINATION bin)
