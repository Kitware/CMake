BOOL
----

Syntax
^^^^^^

.. code-block:: cmake

  $<BOOL:string>

Converts ``string`` to ``0`` or ``1``. Evaluates to ``0`` if any of the
following is true:

* ``string`` is empty,
* ``string`` is a case-insensitive equal of
  ``0``, ``FALSE``, ``OFF``, ``N``, ``NO``, ``IGNORE``, or ``NOTFOUND``, or
* ``string`` ends in the suffix ``-NOTFOUND`` (case-sensitive).

Otherwise evaluates to ``1``.
