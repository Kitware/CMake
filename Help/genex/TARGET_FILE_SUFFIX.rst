TARGET_FILE_SUFFIX
------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_FILE_SUFFIX:tgt>

.. versionadded:: 3.15

Suffix of the ``tgt`` filename (extension such as ``.so`` or ``.exe``).

See also the :prop_tgt:`SUFFIX` target property.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
