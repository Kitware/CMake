enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)

add_library(simple_lib SHARED simple_lib.cpp)
add_executable(app_with_qt app.cpp app_qt.cpp)
target_link_libraries(app_with_qt PRIVATE simple_lib Qt${with_qt_version}::Core)

if(${with_qt_version}Widgets_DIR)
  if(with_qt_version STREQUAL 5)
    qt5_wrap_ui(_headers MyWindow.ui)
  else()
    qt_wrap_ui(_headers MyWindow.ui)
  endif()
  add_executable(app_with_widget app.cpp MyWindow.cpp ${_headers})
  target_link_libraries(app_with_widget PRIVATE Qt${with_qt_version}::Widgets)
  target_include_directories(app_with_widget PRIVATE "${CMAKE_BINARY_DIR}")
endif()
add_subdirectory(QtSubDir1)
add_subdirectory(QtSubDir2)
add_subdirectory(QtSubDir3)
