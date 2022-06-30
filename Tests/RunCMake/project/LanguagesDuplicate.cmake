cmake_policy(SET CMP0057 NEW)

project(ProjectA C C C)
project(ProjectB C C CXX CXX)

get_property(langs GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(lang C CXX)
  if(NOT lang IN_LIST langs)
    message(FATAL_ERROR "Expected language '${lang}' to be enabled.")
  endif()
endforeach()
