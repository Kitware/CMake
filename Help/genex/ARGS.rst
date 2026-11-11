ARGS
----

Syntax
^^^^^^

.. code-block:: cmake

  $<ARGS:cmake-list>

.. versionadded:: 4.5

Transform a :ref:`semicolon-separated list <CMake Language Lists>` into a
comma-separated list as expected by many generator expressions.

.. code-block:: cmake

  # check if the current config is a debug one

  ## Converting a cmake list by hand does not give the expected result
  $<CONFIG:$<LIST:JOIN,$<GLOBAL_PROPERTY:DEBUG_CONFIGURATIONS>,$<COMMA>>>

  ## correct way to proceed
  $<CONFIG:$<ARGS:$<GLOBAL_PROPERTY:DEBUG_CONFIGURATIONS>>>
