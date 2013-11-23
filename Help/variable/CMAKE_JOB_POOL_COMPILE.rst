CMAKE_JOB_POOL_COMPILE
----------------------

Job pool used for compiling.

If this variable is set to a pool name defined by the global
:prop_gbl:`JOB_POOLS` property,
this pool is used for compling without explicitely setting
the the target property :prop_tgt:`JOB_POOL_COMPILE`.

Setting :prop_tgt:`JOB_POOL_COMPILE` on a target overwrites
:variable:`CMAKE_JOB_POOL_COMPILE`.
