LINK_ONLY
---------

.. |genex-family| replace:: :ref:`genex-family-link-context`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<LINK_ONLY:...>

.. versionadded:: 3.1

Content of ``...``, except while collecting usage requirements from
:ref:`transitive compile properties <Transitive Compile Properties>`, in which
case it is the empty string.  This is intended for use in an
:prop_tgt:`INTERFACE_LINK_LIBRARIES` target property, typically populated via
the :command:`target_link_libraries` command, to specify private link
dependencies without other usage requirements such as include directories or
compile options.

.. versionadded:: 3.24
  ``LINK_ONLY`` may also be used in a :prop_tgt:`LINK_LIBRARIES` target
  property.  See policy :policy:`CMP0131`.
