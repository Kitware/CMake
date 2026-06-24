JOB_POOL_COMPILE
----------------

.. versionadded:: 4.5

:ref:`Ninja only <Ninja Generators>`: Pool used for compiling.

The number of parallel compile processes for a rule may be limited by defining
pools with the global :prop_gbl:`JOB_POOLS` property and then specifying the
pool to use.

See Also
^^^^^^^^

* :prop_tgt:`JOB_POOL_COMPILE` target property
* :prop_fs:`JOB_POOL_COMPILE` file set property
* :prop_sf:`JOB_POOL_COMPILE` source file property
