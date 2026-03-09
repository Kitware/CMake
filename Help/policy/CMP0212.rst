CMP0212
-------

.. versionadded:: 4.4

:command:`add_custom_command` ``DEPENDS`` does not strip ``.exe`` suffixes.

When searching for dependencies specified with the ``DEPENDS`` option of
:command:`add_custom_command`, CMake 4.3 and below applied a heuristic to
remove ``.exe`` from the end of ``DEPENDS`` argument values, which created a
target-level dependency on an executable with that stripped name if such a
target exists. This allowed using the name of the target output file as an
alternative spelling when naming dependencies. However, with the emergence of
CMake's target model, specifying the output file in this way is no longer
needed, and leads to conflicts in creating target- and file-level dependencies
when targets exist with the ``.exe`` suffix in the name itself.

The ``OLD`` behavior of this policy strips the ``.exe`` suffix from arguments
to ``DEPENDS`` when searching for target-level dependencies. The ``NEW``
behavior does not strip the ``.exe`` suffix in its search. CMake will otherwise
proceed normally with the other dependency search heuristics as specified in
:command:`add_custom_command` ``DEPENDS``. Users are encouraged to specify the
name of the executable target if a target-level dependency is desired, or use
the :genex:`TARGET_FILE` generator expression if a file-level dependency
is desired, rather than implying the target indirectly via its output file name.

.. |INTRODUCED_IN_CMAKE_VERSION| replace:: 4.4
.. |WARNS_OR_DOES_NOT_WARN| replace:: does *not* warn
.. include:: include/STANDARD_ADVICE.rst

.. include:: include/DEPRECATED.rst
