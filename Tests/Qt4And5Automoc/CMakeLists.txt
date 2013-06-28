
project(Qt4And5Automoc)

find_package(Qt4 REQUIRED)
find_package(Qt5Core REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(qt4_exe main_qt4.cpp)
target_link_libraries(qt4_exe Qt4::QtCore)
add_executable(qt5_exe main_qt5.cpp)
target_link_libraries(qt5_exe Qt5::Core)
