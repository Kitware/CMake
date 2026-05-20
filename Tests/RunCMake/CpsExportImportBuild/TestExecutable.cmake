project(TestLibrary C)

set(liba_DIR "${CMAKE_BINARY_DIR}/../TestLibrary-build/cps/liba")
set(libb_DIR "${CMAKE_BINARY_DIR}/../TestLibrary-build/cps/libb")

find_package(libb REQUIRED COMPONENTS libb)

add_executable(app app.c)

target_link_libraries(app PUBLIC libb::libb)

install(TARGETS app DESTINATION bin)
