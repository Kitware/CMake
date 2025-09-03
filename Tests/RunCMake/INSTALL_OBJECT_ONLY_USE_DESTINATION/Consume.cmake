find_package(IOOUD CONFIG REQUIRED)

add_executable(main main.c)
target_link_libraries(main PRIVATE IOOUD::objlib)

enable_testing()
add_test(NAME run COMMAND main)
