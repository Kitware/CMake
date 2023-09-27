enable_language(CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

add_library(dummy STATIC example.cpp)
target_link_libraries(dummy Qt${with_qt_version}::Core
                            Qt${with_qt_version}::Widgets
                            Qt${with_qt_version}::Gui)

set_target_properties(dummy PROPERTIES AUTOMOC ON)
