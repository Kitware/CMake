PATH:APPEND
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:APPEND,path...,input,...>

.. versionadded:: 3.24

Returns all the ``input`` arguments appended to ``path`` using ``/`` as the
``directory-separator``. Depending on the ``input``, the value of ``path`` may
be discarded.

See :command:`cmake_path(APPEND)` for more details.
