include(apple-common.cmake)

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
find_package(mylib CONFIG REQUIRED)

add_executable(myexe src/myexe.c)
target_link_libraries(myexe PRIVATE mylib)
