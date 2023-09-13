cmake_policy(SET CMP0116 NEW)

add_executable(sub_exe_2 ../app.cpp)
target_link_libraries(sub_exe_2 PRIVATE Qt${with_qt_version}::Core)
