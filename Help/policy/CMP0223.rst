CMP0223
-------

.. versionadded:: 4.5

An empty path is not a prefix of any path.

:command:`cmake_path(IS_PREFIX)` and the ``$<PATH:IS_PREFIX>`` generator
expression treat an empty path as a prefix of every path, including
another empty path.  A prefix that is empty because a variable was set to
an empty value, or because a generator expression argument expanded to
nothing, therefore satisfies a check that was meant to reject it.

The :command:`if` command's ``PATH_IS_PREFIX`` operator is new in the same
release and has no previous behavior of its own, but it follows this
policy so that it agrees with :command:`cmake_path(IS_PREFIX)` in every
policy state.

The ``OLD`` behavior for this policy is to treat an empty path as a prefix
of every path.  The ``NEW`` behavior is to treat an empty path as a prefix
of no path.

.. |INTRODUCED_IN_CMAKE_VERSION| replace:: 4.5
.. |WARNS_OR_DOES_NOT_WARN| replace:: warns
.. include:: include/STANDARD_ADVICE.rst

.. include:: include/DEPRECATED.rst
