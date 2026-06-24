GLOBAL
------

.. versionadded:: 4.5

Indication of whether a rule is globally visible.

The boolean value of this property is true for rules created with the
``GLOBAL`` options to :command:`add_custom_rule()`.

For rules created without the additional option ``GLOBAL`` this is false.
However, setting this property to true promotes that rule to global scope. This
promotion can only be done in the same directory where the rule was created.

.. note::

  Once an rule has been made global, it cannot be changed back to
  non-global. Therefore, if a project sets this property, it may only
  provide a value of true. CMake will issue an error if the project tries to
  set the property to a non-true value, even if the value was already false.
