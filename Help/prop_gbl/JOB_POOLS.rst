JOB_POOLS
---------

Ninja only: List of available pools.

A pool is a named integer property and defines the maximum number
of concurrent jobs which can be started by a rule assigned to the pool.
The JOB_POOLS property is a semicolon-separated list of pairs using
the syntax NAME=integer.

For instance set_property(GLOBAL PROPERTY POOLS "two_jobs=2;ten_jobs=10").

Defined pools could be used globally by setting CMAKE_JOB_POOL_COMPILE/LINK
or per target by setting the target property JOB_POOL_COMPILE/LINK.
