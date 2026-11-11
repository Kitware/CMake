PATH:NATIVE_PATH
----------------

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:NATIVE_PATH[,NORMALIZE],path...>

.. versionadded:: 4.0

Returns ``path`` converted into a native format with platform-specific slashes
(``\`` on Windows hosts and ``/`` elsewhere).

When the ``NORMALIZE`` option is specified, the path is :ref:`normalized
<Normalization>` before the conversion.
