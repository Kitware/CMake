DEVICE_LINK
-----------

.. |genex-family| replace:: :ref:`genex-family-link-context`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<DEVICE_LINK:list>

.. versionadded:: 3.18

Returns the list if it is the device link step, an empty list otherwise. The
device link step is controlled by :prop_tgt:`CUDA_SEPARABLE_COMPILATION` and
:prop_tgt:`CUDA_RESOLVE_DEVICE_SYMBOLS` properties and policy :policy:`CMP0105`.
This expression can only be used to specify link options.
