default-generator-env
---------------------

* The :envvar:`CMAKE_GENERATOR` environment variable was added
  to specify a default generator to use when :manual:`cmake(1)` is
  run without a ``-G`` option.  Additionally, environment variables
  :envvar:`CMAKE_GENERATOR_PLATFORM`, :envvar:`CMAKE_GENERATOR_TOOLSET`,
  and :envvar:`CMAKE_GENERATOR_INSTANCE` were created to configure
  the generator.
