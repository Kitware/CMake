RULE_PROPERTY
-------------

.. |genex-family| replace:: :ref:`genex-family-generator-expression-reference`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<RULE_PROPERTY:rule,prop>

.. versionadded:: 4.5

Value of the property ``prop`` on the rule ``rule``, or empty if the property is
not set. An error will be raised if the rule is not known by CMake.

This generator expression can only be used in the definition of a custom rule
(see :command:`add_custom_rule`). Moreover, ``rule`` parameter must be the
pattern ``<RULE>``. Any other value will raise an error.
