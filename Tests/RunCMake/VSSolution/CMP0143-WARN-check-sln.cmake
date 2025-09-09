getProjectNames(projects)

list(FIND projects "CMakePredefinedTargets" found)
  if(NOT (found EQUAL "-1"))
    error("CMakePredefinedTargets should not be defined when CMP0143 is OLD!")
endif()
