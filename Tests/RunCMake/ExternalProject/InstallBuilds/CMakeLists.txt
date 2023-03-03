cmake_minimum_required(VERSION 3.24)
project(InstallBuilds NONE)
add_custom_target(always ALL COMMAND ${CMAKE_COMMAND} -E echo "always builds")
install(CODE "")
