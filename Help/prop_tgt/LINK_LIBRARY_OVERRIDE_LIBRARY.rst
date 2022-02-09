LINK_LIBRARY_OVERRIDE_<LIBRARY>
-------------------------------

.. versionadded:: 3.24

To resolve incompatible features introduced by :genex:`LINK_LIBRARY` generator
expression, this property offers the possibility to override, for a
``link-item`` (``CMake`` target or external library) involved in the link step,
any defined features with a new one.

This property takes a ``feature`` name which will be applied to the
``link-item`` specified by ``<LIBRARY>`` suffix property. For example:

.. code-block:: cmake

  add_library(lib1 ...)
  target_link_libraries(lib1 PUBLIC $<LINK_LIBRARY:feature1,external>)

  add_library(lib2 ...)
  target_link_libraries(lib2 PUBLIC $<LINK_LIBRARY:feature2,lib1>)

  add_library(lib3 ...)
  target_link_libraries(lib3 PRIVATE lib1 lib2)
  # Here, lib1 has two different features which prevents to link lib3
  # So, define LINK_LIBRARY_OVERRIDE_lib1 property to ensure correct link
  set_property(TARGET lib3 PROPERTY LINK_LIBRARY_OVERRIDE_lib1 feature2)
  # The lib1 will be used with feature2 to link lib3

It is also possible to override any feature with the pre-defined feature
``DEFAULT`` to get the standard behavior (i.e. no feature):

.. code-block:: cmake

  set_property(TARGET lib3 PROPERTY LINK_LIBRARY_OVERRIDE_lib1 DEFAULT)
  # The lib1 will be used without any feature to link lib3

Contents of ``LINK_LIBRARY_OVERRIDE_<LIBRARY>`` may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.

This property takes precedence over :prop_tgt:`LINK_LIBRARY_OVERRIDE`
target property.

For more information about features, see
:variable:`CMAKE_<LANG>_LINK_USING_<FEATURE>`
and :variable:`CMAKE_LINK_USING_<FEATURE>` variables.
