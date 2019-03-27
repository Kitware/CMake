project(ProjectInclude LANGUAGES NONE)
if(NOT AUTO_INCLUDE)
  message(FATAL_ERROR "include file not found")
endif()
