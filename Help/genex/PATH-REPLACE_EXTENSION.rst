PATH:REPLACE_EXTENSION
----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:REPLACE_EXTENSION[,LAST_ONLY],path...,input>

.. versionadded:: 3.24

Returns ``path`` with the :ref:`extension <EXTENSION_DEF>` replaced by
``input``, if any.

See :command:`cmake_path(REPLACE_EXTENSION)` for more details.
