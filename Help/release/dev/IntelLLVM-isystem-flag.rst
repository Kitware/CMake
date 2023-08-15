IntelLLVM-isystem-flag
----------------------

* The :generator:`Ninja` and :generator:`NMake Makefiles` generators now use
  the ``-external:I`` flag for system includes when using IntelLLVM as of
  version 2021.4. The ``-external:W0`` flag is also used as of version 2022.2.
