enable_language(CXX)

find_package(Qt5Core REQUIRED)

set(CMAKE_AUTOMOC ON)

add_library(simple_lib SHARED simple_lib.cpp)
add_executable(app_with_qt app.cpp app_qt.cpp)
target_link_libraries(app_with_qt PRIVATE simple_lib Qt5::Core)

add_subdirectory(QtSubDir1)
add_subdirectory(QtSubDir2)
add_subdirectory(QtSubDir3)
