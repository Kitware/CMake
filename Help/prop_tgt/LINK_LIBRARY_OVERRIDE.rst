LINK_LIBRARY_OVERRIDE
---------------------

.. versionadded:: 3.24

To resolve incompatible features introduced by :genex:`LINK_LIBRARY` generator
expression, this property offers the possibility to override, per ``link-item``
(``CMake`` target or external library) involved in the link step, any defined
features with a new one.

This property takes a :ref:`;-list <CMake Language Lists>` of override
declarations which have the following format:

::

  feature[,link-item]*

For the list of ``link-item`` (``CMake`` target or external library) specified,
the feature ``feature`` will be used in place of any declared feature. For
example:

.. code-block:: cmake

  add_library(lib1 ...)
  target_link_libraries(lib1 PUBLIC "$<LINK_LIBRARY:feature1,external>")

  add_library(lib2 ...)
  target_link_libraries(lib2 PUBLIC "$<LINK_LIBRARY:feature2,lib1>")

  add_library(lib3 ...)
  target_link_libraries(lib3 PRIVATE lib1 lib2)
  # Here, lib1 has two different features which prevents to link lib3
  # So, define LINK_LIBRARY_OVERRIDE property to ensure correct link
  set_property(TARGET lib3 PROPERTY LINK_LIBRARY_OVERRIDE "feature2,lib1,external")
  # The lib1 and external will be used with FEATURE2 to link lib3

It is also possible to override any feature with the pre-defined feature
``DEFAULT`` to get the standard behavior (i.e. no feature):

.. code-block:: cmake

  set_property(TARGET lib3 PROPERTY LINK_LIBRARY_OVERRIDE "DEFAULT,lib1"
                                                          "feature2,external")
  # The lib1 will be used without any feature and external will use feature2 to link lib3

Contents of ``LINK_LIBRARY_OVERRIDE`` may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.

See also :prop_tgt:`LINK_LIBRARY_OVERRIDE_<LIBRARY>` target property for
a per linked target oriented approach to override features.

For more information about features, see
:variable:`CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>`
and :variable:`CMAKE_LINK_LIBRARY_USING_<FEATURE>` variables.
