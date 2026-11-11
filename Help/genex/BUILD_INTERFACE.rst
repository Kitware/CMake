BUILD_INTERFACE
---------------

Syntax
^^^^^^

.. code-block:: cmake

  $<BUILD_INTERFACE:...>

Content of ``...`` when the property is exported using :command:`export`, or
when the target is used by another target in the same buildsystem. Expands to
the empty string otherwise.
