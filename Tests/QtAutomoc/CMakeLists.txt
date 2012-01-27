cmake_minimum_required(VERSION 2.8)

project(QtAutomoc)

find_package(Qt4 REQUIRED)

include(UseQt4)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DFOO)

# enable relaxed mode so automoc can handle all the special cases:
set(CMAKE_AUTOMOC_RELAXED_MODE TRUE)

# create an executable and a library target, both requiring automoc:
add_library(codeeditorLib STATIC codeeditor.cpp)

add_executable(foo main.cpp calwidget.cpp foo.cpp blub.cpp bar.cpp abc.cpp xyz.cpp yaf.cpp private_slot.cpp)

set_target_properties(foo codeeditorLib PROPERTIES AUTOMOC TRUE)

target_link_libraries(foo codeeditorLib ${QT_LIBRARIES} )
