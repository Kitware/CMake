cmake_minimum_required(VERSION 2.6)
project(VSExcludeFromDefaultBuild)

add_executable(DefaultBuilt main.c)

add_executable(AlwaysBuilt main.c)
set_target_properties(AlwaysBuilt PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD FALSE)

add_executable(NeverBuilt main.c)
set_target_properties(NeverBuilt PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)

foreach(config ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${config} Config)
  add_executable(BuiltIn${config} main.c)
  set_target_properties(BuiltIn${config} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE EXCLUDE_FROM_DEFAULT_BUILD_${Config} FALSE)
  add_executable(ExcludedIn${config} main.c)
  set_target_properties(ExcludedIn${config} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_${Config} TRUE)
endforeach()
