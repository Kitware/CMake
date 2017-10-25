cmake_minimum_required(VERSION 3.9)

list(APPEND CMAKE_AUTOMOC_MACRO_NAMES "QO1_ALIAS")

add_executable(mmn main.cpp Gadget.cpp Object.cpp Object1Aliased.cpp Object2Aliased.cpp)
set_property(TARGET mmn PROPERTY AUTOMOC ON)
set_property(TARGET mmn APPEND PROPERTY AUTOMOC_MACRO_NAMES "QO2_ALIAS")
target_link_libraries(mmn ${QT_LIBRARIES})
