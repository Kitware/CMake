getFirstProject(first_project StartupProjectMissing)
if(NOT first_project STREQUAL "ALL_BUILD")
  error("ALL_BUILD is not the startup project")
endif()
