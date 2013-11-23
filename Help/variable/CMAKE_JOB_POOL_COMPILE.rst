CMAKE_JOB_POOL_LINK
-------------------

Job pool used for linking.

If this variable is set to a pool name defined in JOB_POOLS,
this pool is used for linking without explicitely setting
the the target property JOB_POOL_LINK.
Setting JOB_POOL_LINK on a target overwrites CMAKE_JOB_POOL_LINK.
