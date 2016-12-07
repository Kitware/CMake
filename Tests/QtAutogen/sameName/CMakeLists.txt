# Test AUTOMOC and AUTORCC on source files with the same name
# but in different subdirectories

add_executable(sameName
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
  item.cpp
  data.qrc
  main.cpp
)
target_link_libraries(sameName ${QT_LIBRARIES})
set_target_properties(sameName PROPERTIES AUTOMOC TRUE AUTORCC TRUE)
