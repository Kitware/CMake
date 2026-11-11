TARGET_POLICY
-------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_POLICY:policy>

``1`` if the ``policy`` was ``NEW`` when the 'head' target was created, else
``0``.  If the ``policy`` was not set, the warning message for the policy will
be emitted. This generator expression only works for a subset of policies.
