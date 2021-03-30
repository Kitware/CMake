enable_language(CXX)

find_package(Qt5Core REQUIRED)

set(CMAKE_AUTOMOC ON)

add_library(simple_lib SHARED simple_lib.cpp)
add_executable(app_with_qt app.cpp app_qt.cpp)
target_link_libraries(app_with_qt PRIVATE simple_lib Qt5::Core)

if(Qt5Widgets_DIR)
  find_package(Qt5Widgets REQUIRED)
  qt5_wrap_ui(_headers MyWindow.ui)
  add_executable(app_with_widget app.cpp MyWindow.cpp ${_headers})
  target_link_libraries(app_with_widget PRIVATE Qt5::Widgets)
  target_include_directories(app_with_widget PRIVATE "${CMAKE_BINARY_DIR}")
endif()

add_subdirectory(QtSubDir1)
add_subdirectory(QtSubDir2)
add_subdirectory(QtSubDir3)
