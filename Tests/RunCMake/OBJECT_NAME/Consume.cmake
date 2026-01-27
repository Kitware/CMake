find_package(ON CONFIG REQUIRED)

add_executable(main main.c)
target_link_libraries(main PRIVATE ON::objlib)

enable_testing()
add_test(NAME run COMMAND main)
