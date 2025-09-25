if(DEFINED case)
  set(PROJECT_NAME ${case})
  include(${case}.cmake)
else()
  # If no case is defined it is an initial setup
  # Start with the same setup as UsrLocal
  set(PROJECT_NAME UsrLocal)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
  include(Common.cmake)
endif()
