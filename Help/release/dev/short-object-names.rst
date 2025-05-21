short-object-names
------------------

* There is now the :variable:`CMAKE_INTERMEDIATE_DIR_STRATEGY` variable (and
  associated environment variable :envvar:`CMAKE_INTERMEDIATE_DIR_STRATEGY`)
  that may be used to change the strategy used to name intermediate
  directories used for object files (and other associated target metadata). It
  is supported for the following generators:

  - :ref:`Ninja Generators`
