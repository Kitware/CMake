enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)

add_library(dummy SHARED empty.cpp)
target_link_libraries(dummy Qt${with_qt_version}::Core
                            Qt${with_qt_version}::Widgets
                            Qt${with_qt_version}::Gui)
