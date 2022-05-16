CMAKE_PROJECT_TOP_LEVEL_INCLUDES
--------------------------------

* The :variable:`CMAKE_PROJECT_TOP_LEVEL_INCLUDES` variable was added to allow
  injecting custom code at the site of the first :command:`project` call,
  after the host and target platform details have been determined.
