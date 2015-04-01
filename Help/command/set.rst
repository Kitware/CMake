set
---

Set a normal, cache, or environment variable to a given value.
See the :ref:`cmake-language(7) variables <CMake Language Variables>`
documentation for the scopes and interaction of normal variables
and cache entries.

Signatures of this command that specify a ``<value>...`` placeholder
expect zero or more arguments.  Multiple arguments will be joined as
a :ref:`;-list <CMake Language Lists>` to form the actual variable
value to be set.  Zero arguments will cause normal variables to be
unset.  See the :command:`unset` command to unset variables explicitly.

Set Normal Variable
^^^^^^^^^^^^^^^^^^^

::

  set(<variable> <value>... [PARENT_SCOPE])

Set the given ``<variable>`` in the current function or directory scope.

If the ``PARENT_SCOPE`` option is given the variable will be set in
the scope above the current scope.  Each new directory or function
creates a new scope.  This command will set the value of a variable
into the parent directory or calling function (whichever is applicable
to the case at hand).

Set Cache Entry
^^^^^^^^^^^^^^^

::

  set(<variable> <value>... CACHE <type> <docstring> [FORCE])

Set the given cache ``<variable>`` (cache entry).  Since cache entries
are meant to provide user-settable values this does not overwrite
existing cache entries by default.  Use the ``FORCE`` option to
overwrite existing entries.

The ``<type>`` must be specified as one of:

``BOOL``
  Boolean ``ON/OFF`` value.  :manual:`cmake-gui(1)` offers a checkbox.

``FILEPATH``
  Path to a file on disk.  :manual:`cmake-gui(1)` offers a file dialog.

``PATH``
  Path to a directory on disk.  :manual:`cmake-gui(1)` offers a file dialog.

``STRING``
  A line of text.  :manual:`cmake-gui(1)` offers a text field or a
  drop-down selection if the :prop_cache:`STRINGS` cache entry
  property is set.

``INTERNAL``
  A line of text.  :manual:`cmake-gui(1)` does not show internal entries.
  They may be used to store variables persistently across runs.
  Use of this type implies ``FORCE``.

The ``<docstring>`` must be specified as a line of text providing
a quick summary of the option for presentation to :manual:`cmake-gui(1)`
users.

If the cache entry does not exist prior to the call or the ``FORCE``
option is given then the cache entry will be set to the given value.
Furthermore, any normal variable binding in the current scope will
be removed to expose the newly cached value to any immediately
following evaluation.

Set Environment Variable
^^^^^^^^^^^^^^^^^^^^^^^^

::

  set(ENV{<variable>} <value>...)

Set the current process environment ``<variable>`` to the given value.
