TYPE
----

.. versionadded:: 4.4

The type of the file set.

This read-only property can be used to retrieve the
:ref:`type <File Sets>` of the given file set.
Possible values are ``CXX_MODULES``, ``SOURCES``, or ``HEADERS``.

.. versionchanged:: 4.5
  The experimental ``SYCL`` and ``SYCL_HEADERS`` types accepted by
  :command:`target_sources` are stored as ``SOURCES`` and ``HEADERS``,
  respectively, with the :prop_fs:`LANGUAGE` property set to ``SYCL``.
  Therefore, this property reports the canonical type for file sets created
  with either alias.  Both aliases are gated by ``CMAKE_EXPERIMENTAL_SYCL``.
