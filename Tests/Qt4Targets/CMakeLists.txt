cmake_minimum_required(VERSION 2.8)

project(Qt4Targets)

find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES})

add_executable(Qt4Targets WIN32 main.cpp)
target_link_libraries(Qt4Targets Qt4::QtDeclarative)

if (WIN32)
  target_link_libraries(Qt4Targets Qt4::qtmain)
endif()
