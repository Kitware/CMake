cmake_minimum_required(VERSION 3.18)
project(cxx-as-objcxx LANGUAGES OBJCXX)

add_executable(cxx-as-objcxx main.cpp)
set_source_files_properties(main.cpp PROPERTIES LANGUAGE OBJCXX)
