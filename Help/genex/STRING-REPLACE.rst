STRING:REPLACE
--------------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:REPLACE[,(STRING|REGEX)],string,match_string,replace_string>

.. versionadded:: 4.3

Replace all occurrences of ``match_string`` in the ``string`` with
``replace_string``.

The ``match_string`` can be of two different types:

``STRING``
  ``match_string`` is a literal string and match will be done by simple
  string comparison. This is the default.

``REGEX``
  ``match_string`` is a regular expression. Match this regular_expression as
  many times as possible and substitute the ``replace_string`` for the match
  in the ``string``.

  The ``replace_string`` may refer to parenthesis-delimited subexpressions of
  the match using \\1, \\2, ..., \\9. Note that two backslashes (\\\\1) are
  required in CMake code to get a backslash through argument parsing.
