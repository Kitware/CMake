cmake_minimum_required(VERSION 3.16.0)
project(SameName)
include("../AutogenGuiTest.cmake")

# Process .hh headers in AUTOMOC
cmake_policy(SET CMP0100 NEW)

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
  object.h
  object.hh
  object.h++
  object.hpp
  object.hxx
  object_upper_ext.H
  data.qrc
  main.cpp
)
target_link_libraries(sameName ${QT_LIBRARIES})
set_target_properties(sameName PROPERTIES
  AUTOMOC TRUE
  AUTOUIC TRUE
  AUTORCC TRUE
)

# Set different compression levels
if (QT_TEST_VERSION EQUAL 4)
  set(rccCompress "-compress")
else()
  set(rccCompress "--compress")
endif()
set_target_properties(sameName PROPERTIES AUTORCC_OPTIONS "${rccCompress};1" )
set_source_files_properties(aaa/data.qrc PROPERTIES AUTORCC_OPTIONS "${rccCompress};2" )
set_source_files_properties(bbb/data.qrc PROPERTIES AUTORCC_OPTIONS "${rccCompress};3" )
set_source_files_properties(ccc/data.qrc PROPERTIES AUTORCC_OPTIONS "${rccCompress};4" )
