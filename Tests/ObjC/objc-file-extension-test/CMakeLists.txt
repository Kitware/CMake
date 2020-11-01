cmake_minimum_required(VERSION 3.15)

project(objc-file-extension-test OBJC CXX)

add_executable(objc-file-extension-test main.m)
target_link_libraries(objc-file-extension-test "-framework Foundation")
