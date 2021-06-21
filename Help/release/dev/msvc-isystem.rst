msvc-isystem
------------

* The MSVC compilers learned to pass the ``-external:I`` flag for system
  includes when using the :generator:`Ninja` and :generator:`NMake Makefiles`
  generators. This became available as of Visual Studio 16.10 (toolchain
  version 14.29.30037).
