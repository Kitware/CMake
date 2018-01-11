extend-compile-language-genex
-----------------------------

* :ref:`Visual Studio Generators` learned to support the ``COMPILE_LANGUAGE``
  :manual:`generator expression <cmake-generator-expressions(7)>` in
  target-wide :prop_tgt:`COMPILE_DEFINITIONS`,
  :prop_tgt:`COMPILE_OPTIONS`, and :command:`file(GENERATE)`.

* The :generator:`Xcode` generator learned to support the ``COMPILE_LANGUAGE``
  :manual:`generator expression <cmake-generator-expressions(7)>` in
  target-wide :prop_tgt:`COMPILE_DEFINITIONS`.
  It previously supported only :prop_tgt:`COMPILE_OPTIONS` and
  :command:`file(GENERATE)`.
