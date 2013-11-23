JOB_POOL_COMPILE
----------------

Ninja only: Pool used for compiling.

The number of parallel compile processes could be limited by defining
pools with the global JOB_POOLS property and then specifing here the pool name.

For instance set_target_properties(target PROPERTIES JOB_POOL_COMPILE ten_jobs)

This property overwrites the variable CMAKE_JOB_POOL_COMPILE.
