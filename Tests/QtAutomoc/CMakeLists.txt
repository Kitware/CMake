cmake_minimum_required(VERSION 2.8)

project(QtAutomoc)

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)

  include(UseQt4)

  set(QT_QTCORE_TARGET Qt4::QtCore)
else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()
  find_package(Qt5Widgets REQUIRED)

  set(QT_QTCORE_TARGET Qt5::Core)

  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  set(QT_LIBRARIES Qt5::Widgets)

  if(Qt5_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()
endif()


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
# headers both subdirectories use CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE and we
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

add_library(empty STATIC empty.cpp)
set_target_properties(empty PROPERTIES AUTOMOC TRUE)
target_link_libraries(empty no_link_language)
add_library(no_link_language STATIC empty.h)
set_target_properties(no_link_language PROPERTIES AUTOMOC TRUE)
