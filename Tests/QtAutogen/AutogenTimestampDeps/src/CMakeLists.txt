add_executable(Exe main.cpp ${CMAKE_BINARY_DIR}/ProjectInfo.hpp)
add_dependencies(Exe ProjectInfo)
target_include_directories(Exe PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(Exe PRIVATE ${QT_QTCORE_TARGET})
