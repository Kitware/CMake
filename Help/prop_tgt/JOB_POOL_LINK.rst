JOB_POOL_LINK
-------------

Ninja only: Pool used for linking.

The number of parallel link processes could be limited by defining
pools with the global JOB_POOLS property and then specifing here the pool name.
For instance set_target_properties(target PROPERTIES JOB_POOL_LINK two_jobs)
This property overwrites the variable CMAKE_JOB_POOL_LINK.
