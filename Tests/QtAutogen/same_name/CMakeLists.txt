# Test AUTOMOC and AUTORCC on source files with the same name
# but in different subdirectories

add_executable(same_name
  aaa/bbb/item.cpp
  aaa/bbb/data.qrc
  aaa/item.cpp
  aaa/data.qrc
  bbb/aaa/item.cpp
  bbb/aaa/data.qrc
  bbb/item.cpp
  bbb/data.qrc
  ccc/item.cpp
  ccc/data.qrc
  main.cpp
  data.qrc
)
target_include_directories(same_name PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(same_name ${QT_LIBRARIES})
set_target_properties( same_name PROPERTIES AUTOMOC TRUE AUTORCC TRUE )
