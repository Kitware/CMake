enable_language(CXX)

set(QtX Qt${with_qt_version})

find_package(${QtX} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)

add_library(simple_lib SHARED simple_lib.cpp)
add_executable(app_with_qt app.cpp app_qt.cpp)

target_link_libraries(app_with_qt PRIVATE simple_lib ${QtX}::Core)

set_source_files_properties(app.cpp app_qt.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_precompile_headers(app_with_qt PRIVATE [["QObject"]])
