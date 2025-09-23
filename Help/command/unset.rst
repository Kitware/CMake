unset
-----

Unset a variable, cache variable, or environment variable.

Unset Normal Variable or Cache Entry
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  unset(<variable> [CACHE | PARENT_SCOPE])

Removes a normal variable from the current scope, causing it
to become undefined.  If ``CACHE`` is present, then a cache variable
is removed instead of a normal variable.

If ``PARENT_SCOPE`` is present then the variable is removed from the scope
above the current scope.  See the same option in the :command:`set` command
for further details.

.. include:: include/UNSET_NOTE.rst

Unset Environment Variable
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  unset(ENV{<variable>})

Removes ``<variable>`` from the currently available
:manual:`Environment Variables <cmake-env-variables(7)>`.
Subsequent calls of ``$ENV{<variable>}`` will return the empty string.

This command affects only the current CMake process, not the process
from which CMake was called, nor the system environment at large,
nor the environment of subsequent build or test processes.

See Also
^^^^^^^^

* :command:`set`
