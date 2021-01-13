external-project-configure-handled-by-build
-------------------------------------------

* The :module:`ExternalProject` function ``ExternalProject_Add`` learned a new
  ``CONFIGURE_HANDLED_BY_BUILD`` option to have subsequent runs of the configure
  step be triggered by the build step when an external project dependency
  rebuilds instead of always rerunning the configure step when an external
  project dependency rebuilds.
