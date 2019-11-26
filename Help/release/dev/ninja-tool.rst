ninja-tool
----------

* The :generator:`Ninja` generator now prefers the first ninja build
  tool to appear in the ``PATH`` no matter whether it is called
  ``ninja-build``, ``ninja``, or ``samu``.  Previously the first
  of those names to appear anywhere in the ``PATH`` would be preferred.
