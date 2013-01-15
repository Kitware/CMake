cmake_minimum_required(VERSION 2.8)

project(QtAutomoc)

find_package(Qt4 REQUIRED)

include(UseQt4)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DFOO -DSomeDefine="Barx")

# enable relaxed mode so automoc can handle all the special cases:
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)

# create an executable and two library targets, each requiring automoc:
add_library(codeeditorLib STATIC codeeditor.cpp)

add_library(privateSlot OBJECT private_slot.cpp)

add_executable(foo main.cpp calwidget.cpp foo.cpp blub.cpp bar.cpp abc.cpp
               xyz.cpp yaf.cpp $<TARGET_OBJECTS:privateSlot>)

set_target_properties(foo codeeditorLib privateSlot PROPERTIES AUTOMOC TRUE)

target_link_libraries(foo codeeditorLib ${QT_LIBRARIES} )
