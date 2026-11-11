STRING:MATCH
------------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:MATCH,string[,SEEK:(ONCE|ALL)],regular_expression>

.. versionadded:: 4.3

Match, in the ``string``, the ``regular_expression``.

The ``SEEK:`` option specifies the match behavior:

``ONCE``
  Match only the first occurrence. This is the default.

``ALL``
  Match as many times as possible and return the matches as a list.

See the :ref:`CMake Language Regex Specification` for the syntax of the
``regular_expression`` parameter.
