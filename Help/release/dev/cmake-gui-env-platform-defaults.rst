cmake-gui-env-platform-defaults
-------------------------------

* :manual:`cmake-gui(1)` now populates its generator selection
  widget default value from the :envvar:`CMAKE_GENERATOR` environment
  variable.  Additionally, environment variables
  :envvar:`CMAKE_GENERATOR_PLATFORM` and :envvar:`CMAKE_GENERATOR_TOOLSET`
  are used to populate their respective widget defaults.
