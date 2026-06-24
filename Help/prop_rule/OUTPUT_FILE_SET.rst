OUTPUT_FILE_SET
---------------

.. versionadded:: 4.5

Specify the name and the type of the file set storing the files produced by the
instantiation of a rule.

The ``OUTPUT_FILE_SET`` property must hold a
:ref:`semicolon-separated list <CMake Language Lists>` of two elements
specifying the name of the file set and the type.

The name can use the following patterns to enable the production of unique
names for the output file set:

* ``<RULE>``: name of the rule
* ``<TARGET>``: name of the target
* ``<FILE_SET>``: name of the file set

The type must be one of the :ref:`predefined types <File Sets>`.

This property gets the following default value:
``__cmake_rule_<RULE>_<TARGET>_<FILE_SET>_outputs;SOURCES``.
