CMAKE_DISABLE_PRECOMPILE_HEADERS
--------------------------------

.. versionadded:: 4.4

.. include:: include/ENV_VAR.rst

The default value for the :variable:`CMAKE_DISABLE_PRECOMPILE_HEADERS` variable
when there is no explicit configuration given on the first run while creating a
new build tree.  On later runs in an existing build tree the value persists in
the cache as :variable:`CMAKE_DISABLE_PRECOMPILE_HEADERS`.
