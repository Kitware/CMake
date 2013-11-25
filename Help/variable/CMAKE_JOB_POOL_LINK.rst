CMAKE_JOB_POOL_LINK
----------------------

Job pool used for linking.

If this variable is set to a pool name defined by the global
:prop_gbl:`JOB_POOLS` property
this pool is used for linking without explicitely setting
the the target property :prop_gbl:`JOB_POOL_LINK`.

Setting :prop_tgt:`JOB_POOL_LINK` on a target overwrites
:variable:`CMAKE_JOB_POOL_LINK`.
