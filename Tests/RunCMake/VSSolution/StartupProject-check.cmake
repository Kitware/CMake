getFirstProject(first_project StartupProject)
if(NOT first_project STREQUAL "TestStartup")
  error("TestStartup is not the startup project")
endif()
