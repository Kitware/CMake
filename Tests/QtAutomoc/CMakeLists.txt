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

include(GenerateExportHeader)
# The order is relevant here. B depends on A, and B headers depend on A
# headers both subdirectories use CMAKE_BUILD_INTERFACE_INCLUDES and we
# test that CMAKE_AUTOMOC successfully reads the include directories
# for the build interface from those targets. There has previously been
# a bug where caching of the include directories happened before
# extracting the includes to pass to moc.
add_subdirectory(Bdir)
add_subdirectory(Adir)
add_library(libC SHARED libC.cpp)
set_target_properties(libC PROPERTIES AUTOMOC TRUE)
generate_export_header(libC)
target_link_libraries(libC LINK_PUBLIC libB)

target_link_libraries(foo codeeditorLib ${QT_LIBRARIES} libC)
