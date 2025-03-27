cmake_minimum_required(VERSION 3.10)

project(UicIncrementalBuild LANGUAGES CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(example
    main.cpp
    ui/widget.cpp
    ui/widget.h
    ui/widget.ui
)

target_link_libraries(example PRIVATE Qt${with_qt_version}::Widgets
                                      Qt${with_qt_version}::Core
                                      Qt${with_qt_version}::Gui)
