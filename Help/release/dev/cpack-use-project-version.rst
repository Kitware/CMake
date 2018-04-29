cpack-use-project-version
-------------------------

* Introduce :variable:`CMAKE_PROJECT_VERSION` and the corresponding components:
  :variable:`CMAKE_PROJECT_VERSION_MAJOR`, :variable:`CMAKE_PROJECT_VERSION_MINOR`,
  :variable:`CMAKE_PROJECT_VERSION_PATCH` and :variable:`CMAKE_PROJECT_VERSION_TWEAK`.

* :module:`CPack` module use :variable:`CMAKE_PROJECT_VERSION_MAJOR`,
  :variable:`CMAKE_PROJECT_VERSION_MINOR` and :variable:`CMAKE_PROJECT_VERSION_PATCH`
  to initialize corresponding CPack variables.
