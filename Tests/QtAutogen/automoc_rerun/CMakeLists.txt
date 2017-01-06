cmake_minimum_required(VERSION 3.7)
project(automoc_rerun CXX)

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)
  set(QT_CORE_TARGET Qt4::QtCore)
else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()

  find_package(Qt5Core REQUIRED)
  set(QT_CORE_TARGET Qt5::Core)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

configure_file(test1.h.in1 test1.h COPYONLY)

add_executable(test1
  ${CMAKE_CURRENT_BINARY_DIR}/test1.h
  test1.cpp
  res1.qrc
  )
target_include_directories(test1 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test1 ${QT_CORE_TARGET})
