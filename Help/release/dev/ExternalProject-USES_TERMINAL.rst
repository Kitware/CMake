ExternalProject-USES_TERMINAL
-----------------------------

* The :module:`ExternalProject` module learned new ``USES_TERMINAL``
  arguments for giving steps exclusive terminal access.  Especially
  useful with the :generator:`Ninja` generator to monitor CMake
  superbuild progress and prevent CPU oversubscription.
