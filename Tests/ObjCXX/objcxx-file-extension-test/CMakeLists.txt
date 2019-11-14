cmake_minimum_required(VERSION 3.15)

project(objcxx-file-extension-test OBJCXX CXX)

add_executable(objcxx-file-extension-test main.mm)
target_link_libraries(objcxx-file-extension-test "-framework Foundation")
