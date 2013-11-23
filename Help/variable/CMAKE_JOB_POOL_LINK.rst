CMAKE_JOB_POOL_COMPILE
----------------------

Job pool used for compiling.

If this variable is set to a pool name defined in JOB_POOLS,
this pool is used for compling without explicitely setting
the the target property JOB_POOL_COMPILING.
Setting JOB_POOL_COMPILING on a target overwrites CMAKE_JOB_POOL_COMPILE.
