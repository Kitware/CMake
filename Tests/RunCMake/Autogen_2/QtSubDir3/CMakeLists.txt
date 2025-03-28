add_executable(sub_exe_3 ../app.cpp)
target_link_libraries(sub_exe_3 PRIVATE Qt${with_qt_version}::Core)
