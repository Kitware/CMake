cmake_minimum_required(VERSION 3.15)

project(TestDifferentComponents LANGUAGES C)

add_subdirectory(subdir)

find_package(Python3 REQUIRED COMPONENTS Development.Module)
