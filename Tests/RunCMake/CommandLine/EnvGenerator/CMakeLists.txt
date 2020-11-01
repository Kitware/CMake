cmake_minimum_required(VERSION 3.14)
project(EnvGenerator C)
if(CMAKE_GENERATOR MATCHES "Visual Studio")
  message(STATUS "CMAKE_VS_PLATFORM_NAME='${CMAKE_VS_PLATFORM_NAME}'")
endif()
add_custom_command(
  OUTPUT output.txt
  COMMAND ${CMAKE_COMMAND} -E echo CustomCommand > output.txt
  )
add_custom_target(CustomTarget ALL DEPENDS output.txt)
