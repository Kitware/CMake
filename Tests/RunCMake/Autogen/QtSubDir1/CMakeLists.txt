cmake_policy(SET CMP0116 OLD)

add_executable(sub_exe_1 ../app.cpp)
target_link_libraries(sub_exe_1 PRIVATE Qt${with_qt_version}::Core)
