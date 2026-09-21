<RULE>_PATTERNS
---------------

.. versionadded:: 4.5

Patterns specification for instantiating the rule ``<RULE>``.

The ``<RULE>_PATTERNS`` property may be set to a
:ref:`semicolon-separated list <CMake Language Lists>` of patterns using the
syntax ``PATTERN=VALUE`` or ``PATTERN=``. More precisely, each item must match
the regular expression ``(^[A-Z][A-Z0-9_]+)=(.*)$``.

CMake will automatically drop any patterns which do not match against this
regular expression.

The list is ordered, so a pattern can use in its definition a previously
defined pattern. In the following example,
``OUTPUT_DIR=/some/path;OUTPUT_FILE=<OUTPUT_DIR>/my_file``, when the pattern
``<OUTPUT_FILE>`` is expanded, the pattern ``<OUTPUT_DIR>`` is already known.

Related properties:

* :prop_fs:`RULE_PATTERNS` to specify patterns for a file set.

Related commands:

* :command:`add_custom_rule` for custom rule specification.
