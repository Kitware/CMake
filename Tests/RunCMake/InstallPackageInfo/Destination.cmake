cmake_path(GET CMAKE_BINARY_DIR PARENT_PATH out_dir)
set(CMAKE_PREFIX_PATH "${out_dir}/install")
set(CMAKE_INSTALL_PREFIX "${out_dir}/install")

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo)
install(PACKAGE_INFO foo DESTINATION test EXPORT foo)
