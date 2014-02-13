
cmake_minimum_required(VERSION 3.0)

project(ConfigSources)

add_executable(ConfigSources
  $<$<CONFIG:Debug>:main.cpp>
  $<$<CONFIG:Release>:does_not_exist.cpp>
)
