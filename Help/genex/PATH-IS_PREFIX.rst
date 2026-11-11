PATH:IS_PREFIX
--------------

Syntax
^^^^^^

.. code-block:: cmake

  $<PATH:IS_PREFIX[,NORMALIZE],path,input>

.. versionadded:: 3.24

Returns ``1`` if ``path`` is the prefix of ``input``, ``0`` otherwise.

When the ``NORMALIZE`` option is specified, ``path`` and ``input`` are
:ref:`normalized <Normalization>` before the check.

An empty ``path`` is not a prefix of any ``input``.

.. versionchanged:: 4.5
  An empty ``path`` was previously a prefix of every ``input``.  See
  policy :policy:`CMP0223`.
