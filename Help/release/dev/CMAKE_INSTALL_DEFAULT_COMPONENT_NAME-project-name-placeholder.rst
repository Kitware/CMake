CMAKE_INSTALL_DEFAULT_COMPONENT_NAME-project-name-placeholder
-------------------------------------------------------------

* :variable:`CMAKE_INSTALL_DEFAULT_COMPONENT_NAME` gained support for a
  special ``<PROJECT_NAME>`` placeholder value.   It tells each
  :command:`install` invocation to take a default component name
  from the current :variable:`PROJECT_NAME` variable value.
