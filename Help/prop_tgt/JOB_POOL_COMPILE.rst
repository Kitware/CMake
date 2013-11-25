JOB_POOL_COMPILE
----------------

Ninja only: Pool used for compiling.

The number of parallel compile processes could be limited by defining
pools with the global :prop_gbl:`JOB_POOLS`
property and then specifinghere the pool name.

For instance:

.. code-block:: cmake

  set_target_properties(target PROPERTIES JOB_POOL_COMPILE ten_jobs)

This property overwrites the variable :variable:`CMAKE_JOB_POOL_COMPILE`.
