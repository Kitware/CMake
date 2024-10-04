cmake_minimum_required(VERSION 3.10)

project(UicIncrementalBuild LANGUAGES CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(example
    src/mainwindow.ui
    src/widget1.ui
    src/widget2.ui
    src/mainwindow.h
    src/widget1.h
    src/widget2.h
    src/main.cpp
    src/mainwindow.cpp
    src/widget1.cpp
    src/widget2.cpp
)

target_link_libraries(example PRIVATE Qt${with_qt_version}::Widgets
                                      Qt${with_qt_version}::Core
                                      Qt${with_qt_version}::Gui)
