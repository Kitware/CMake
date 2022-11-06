getProjectNames(projects)

list(FIND projects "CMakePredefinedTargets" found)
  if(found EQUAL "-1")
    error("CMakePredefinedTargets should be defined when CMP0143 is NEW!")
endif()
