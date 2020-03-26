cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0100 NEW)

add_executable(mocCMP0100New
  ${CSD}/main.cpp
  ${CSD}/Obj.hh # Manually include Obj.hh
  ${CSD}/Obj.cpp
  ${CSD}/Obj2.cpp # Let AUTOMOC detect Obj2.hh
)
target_link_libraries(mocCMP0100New ${QT_LIBRARIES})
