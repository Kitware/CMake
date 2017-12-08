cmake_minimum_required(VERSION 3.10)
project(MocOnly)
include("../AutogenTest.cmake")

# Test different Q_OBJECT position styles
add_executable(mocOnly StyleA.cpp StyleB.cpp main.cpp)
set_property(TARGET mocOnly PROPERTY AUTOMOC ON)
target_link_libraries(mocOnly ${QT_LIBRARIES})
