cmake_minimum_required(VERSION 3.16)
project(ManySources)
include("../AutogenGuiTest.cmake")

# Test AUTOMOC and AUTOUIC on many source files to stress the concurrent
# parsing and processing framework.

set(CSD ${CMAKE_CURRENT_SOURCE_DIR})
set(CBD ${CMAKE_CURRENT_BINARY_DIR})

set(SRCS main.cpp)
set(MAIN_INCLUDES "\n// Item includes\n")
set(MAIN_ITEMS "\n// Items\n")

set(NUM 24)
foreach(III RANGE 1 ${NUM})
  configure_file(${CSD}/object.h.in ${CBD}/object_${III}.h)
  configure_file(${CSD}/item.h.in ${CBD}/item_${III}.h)
  configure_file(${CSD}/item.cpp.in ${CBD}/item_${III}.cpp)
  configure_file(${CSD}/view.ui.in ${CBD}/view_${III}.ui)
  configure_file(${CSD}/data.qrc.in ${CBD}/data_${III}.qrc)

  list(APPEND SRCS ${CBD}/item_${III}.cpp)
  list(APPEND SRCS ${CBD}/data_${III}.qrc)

  string(APPEND MAIN_INCLUDES "#include \"item_${III}.h\"\n")
  string(APPEND MAIN_ITEMS "Item_${III} item_${III};\n")
  string(APPEND MAIN_ITEMS "item_${III}.TheSlot();\n")
endforeach()

configure_file(${CSD}/main.cpp.in ${CBD}/main.cpp)

add_executable(manySources ${SRCS} ${CBD}/main.cpp)
target_link_libraries(manySources ${QT_LIBRARIES})
set_target_properties(manySources PROPERTIES AUTOMOC 1 AUTOUIC 1 AUTORCC 1)
