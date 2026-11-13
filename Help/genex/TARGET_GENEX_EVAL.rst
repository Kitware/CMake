TARGET_GENEX_EVAL
-----------------

.. |genex-family| replace::
   :ref:`genex-family-multi-level-expression-evaluation`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_GENEX_EVAL:tgt,expr>

.. versionadded:: 3.12

Content of ``expr`` evaluated as a generator expression in the context of
``tgt`` target. This enables consumption of custom target properties that
themselves contain generator expressions.

Having the capability to evaluate generator expressions is very useful when you
want to manage custom properties supporting generator expressions. For example:

.. code-block:: cmake

  add_library(foo ...)

  set_property(TARGET foo PROPERTY
    CUSTOM_KEYS $<$<CONFIG:DEBUG>:FOO_EXTRA_THINGS>
  )

  add_custom_target(printFooKeys
    COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:foo,CUSTOM_KEYS>
  )

This naive implementation of the ``printFooKeys`` custom command is wrong
because ``CUSTOM_KEYS`` target property is not evaluated and the content is
passed as is (i.e. ``$<$<CONFIG:DEBUG>:FOO_EXTRA_THINGS>``).

To have the expected result (i.e. ``FOO_EXTRA_THINGS`` if config is ``Debug``),
it is required to evaluate the output of ``$<TARGET_PROPERTY:foo,CUSTOM_KEYS>``:

.. code-block:: cmake

  add_custom_target(printFooKeys
    COMMAND ${CMAKE_COMMAND} -E
      echo $<TARGET_GENEX_EVAL:foo,$<TARGET_PROPERTY:foo,CUSTOM_KEYS>>
  )
