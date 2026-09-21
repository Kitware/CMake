JOB_POOL_COMPILE
----------------

.. versionadded:: 4.4

Ninja only: Pool used for compiling.

The number of parallel compile processes could be limited by defining
pools with the global :prop_gbl:`JOB_POOLS`
property and then specifying here the pool name.

This allows to override the :prop_sf:`JOB_POOL_COMPILE` and
:prop_tgt:`JOB_POOL_COMPILE` values for specific source files within a same
target.

For instance:

.. code-block:: cmake

  set_property(FILE_SET my_fileset TYPE SOURCE PROPERTY JOB_POOL_COMPILE two_jobs)

This property is undefined by default.

See Also
^^^^^^^^

* :prop_rule:`JOB_POOL_COMPILE` rule property
* :prop_sf:`JOB_POOL_COMPILE` source file property
* :prop_tgt:`JOB_POOL_COMPILE` target property
