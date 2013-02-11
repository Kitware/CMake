cmake_minimum_required(VERSION 2.8)

project(Qt4Targets)

cmake_policy(SET CMP0020 NEW)

find_package(Qt4 REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(Qt4Targets WIN32 main.cpp)
target_link_libraries(Qt4Targets Qt4::QtGui)

set_property(TARGET Qt4Targets APPEND PROPERTY
  INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:Qt4::QtGui,INTERFACE_INCLUDE_DIRECTORIES>
)
set_property(TARGET Qt4Targets APPEND PROPERTY
  COMPILE_DEFINITIONS
    $<TARGET_PROPERTY:Qt4::QtGui,INTERFACE_COMPILE_DEFINITIONS>
)

if (WIN32)
  if (TARGET Qt4::QAxServer)
    add_executable(activeqtexe WIN32 activeqtexe.cpp)
    set_property(TARGET activeqtexe PROPERTY QT4_NO_LINK_QTMAIN ON)
    target_link_libraries(activeqtexe Qt4::QAxServer Qt4::QtGui)
    set_property(TARGET activeqtexe APPEND PROPERTY
      INCLUDE_DIRECTORIES
      $<TARGET_PROPERTY:Qt4::QAxServer,INTERFACE_INCLUDE_DIRECTORIES>
      $<TARGET_PROPERTY:Qt4::QtGui,INTERFACE_INCLUDE_DIRECTORIES>
    )
    set_property(TARGET activeqtexe APPEND PROPERTY
      COMPILE_DEFINITIONS
      $<TARGET_PROPERTY:Qt4::QAxServer,INTERFACE_COMPILE_DEFINITIONS>
      $<TARGET_PROPERTY:Qt4::QtGui,INTERFACE_COMPILE_DEFINITIONS>
    )
  endif()
endif()
