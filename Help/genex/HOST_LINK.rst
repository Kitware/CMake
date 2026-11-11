HOST_LINK
---------

Syntax
^^^^^^

.. code-block:: cmake

  $<HOST_LINK:list>

.. versionadded:: 3.18

Returns the list if it is the normal link step, an empty list otherwise. This
expression is mainly useful when a device link step is also involved (see
:genex:`$<DEVICE_LINK:list>` generator expression). This expression can only be
used to specify link options.
