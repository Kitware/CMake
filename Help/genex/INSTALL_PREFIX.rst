INSTALL_PREFIX
--------------

Syntax
^^^^^^

.. code-block:: cmake

  $<INSTALL_PREFIX>

Content of the install prefix when the target is exported via
:command:`install(EXPORT)`, or when evaluated in the
:prop_tgt:`INSTALL_NAME_DIR` property or the ``INSTALL_NAME_DIR`` argument of
:command:`install(RUNTIME_DEPENDENCY_SET)`, and empty otherwise.

.. versionchanged:: 3.27
  Evaluates to the content of the install prefix
  in the code argument of :command:`install(CODE)` or
  the file argument of :command:`install(SCRIPT)`.
