vs_buildcache_support
---------------------

* The :prop_tgt:`VS_NO_COMPILE_BATCHING` target property was added to
  tell :ref:`Visual Studio Generators` whether to disable compiler parallelism
  and call the compiler with one c/cpp file at a time.
