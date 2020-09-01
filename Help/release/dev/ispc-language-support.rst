cmake-ispc-support
------------------


* CMake learned to support ``ISPC`` as a first-class language that can be
  enabled via the :command:`project` and :command:`enable_language` commands.

* ``ISPC`` is currently supported by the :ref:`Makefile Generators`
  and the :generator:`Ninja` generator on Linux, macOS, and Windows.

* The Intel ISPC compiler (``ispc``) is supported.
