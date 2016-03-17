getFirstProject(first_project StartupProject)
if(NOT first_project STREQUAL "ZERO_CHECK")
  error("ZERO_CHECK is not the startup project")
endif()
