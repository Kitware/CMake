LINK_GROUP
----------

Syntax
^^^^^^

.. code-block:: cmake

  $<LINK_GROUP:feature,library-list>

.. versionadded:: 3.24

Specify a group of libraries to link to a target, along with a ``feature`` which
defines how that group should be linked.  For example:

.. code-block:: cmake

  add_library(lib1 STATIC ...)
  add_library(lib2 ...)
  target_link_libraries(lib2 PRIVATE "$<LINK_GROUP:RESCAN,lib1,external>")

This specifies that ``lib2`` should link to ``lib1`` and ``external``, and that
both of those two libraries should be included on the linker command line
according to the definition of the ``RESCAN`` feature.

Feature names are case-sensitive and may only contain letters, numbers and
underscores.  Feature names defined in all uppercase are reserved for CMake's
own built-in features.  Currently, there is only one pre-defined built-in group
feature:

.. include:: ../variable/include/LINK_GROUP_PREDEFINED_FEATURES.rst

Built-in and custom group features are defined in terms of the following
variables:

* :variable:`CMAKE_<LANG>_LINK_GROUP_USING_<FEATURE>_SUPPORTED`
* :variable:`CMAKE_<LANG>_LINK_GROUP_USING_<FEATURE>`
* :variable:`CMAKE_LINK_GROUP_USING_<FEATURE>_SUPPORTED`
* :variable:`CMAKE_LINK_GROUP_USING_<FEATURE>`

The value used for each of these variables is the value as set at the end of the
directory scope in which the target was created.  The usage is as follows:

1. If the language-specific
   :variable:`CMAKE_<LANG>_LINK_GROUP_USING_<FEATURE>_SUPPORTED` variable
   is true, the ``feature`` must be defined by the corresponding
   :variable:`CMAKE_<LANG>_LINK_GROUP_USING_<FEATURE>` variable.
2. If no language-specific ``feature`` is supported, then the
   :variable:`CMAKE_LINK_GROUP_USING_<FEATURE>_SUPPORTED` variable must be
   true and the ``feature`` must be defined by the corresponding
   :variable:`CMAKE_LINK_GROUP_USING_<FEATURE>` variable.

The ``LINK_GROUP`` generator expression is compatible with the
:genex:`LINK_LIBRARY` generator expression. The libraries involved in a group
can be specified using the :genex:`LINK_LIBRARY` generator expression.

Each target or external library involved in the link step is allowed to be part
of multiple groups, but only if all the groups involved specify the same
``feature``.  Such groups will not be merged on the linker command line, the
individual groups will still be preserved.  Mixing different group features for
the same target or library is forbidden.

.. code-block:: cmake

  add_library(lib1 ...)
  add_library(lib2 ...)
  add_library(lib3 ...)
  add_library(lib4 ...)
  add_library(lib5 ...)

  target_link_libraries(lib3 PUBLIC  "$<LINK_GROUP:feature1,lib1,lib2>")
  target_link_libraries(lib4 PRIVATE "$<LINK_GROUP:feature1,lib1,lib3>")
  # lib4 will be linked with the groups {lib1,lib2} and {lib1,lib3}.
  # Both groups specify the same feature, so this is fine.

  target_link_libraries(lib5 PRIVATE "$<LINK_GROUP:feature2,lib1,lib3>")
  # An error will be raised here because both lib1 and lib3 are part of two
  # groups with different features.

When a target or an external library is involved in the link step as part of a
group and also as not part of any group, any occurrence of the non-group link
item will be replaced by the groups it belongs to.

.. code-block:: cmake

  add_library(lib1 ...)
  add_library(lib2 ...)
  add_library(lib3 ...)
  add_library(lib4 ...)

  target_link_libraries(lib3 PUBLIC lib1)

  target_link_libraries(lib4 PRIVATE lib3 "$<LINK_GROUP:feature1,lib1,lib2>")
  # lib4 will only be linked with lib3 and the group {lib1,lib2}

Because ``lib1`` is part of the group defined for ``lib4``, that group then gets
applied back to the use of ``lib1`` for ``lib3``.  The end result will be as
though the linking relationship for ``lib3`` had been specified as:

.. code-block:: cmake

  target_link_libraries(lib3 PUBLIC "$<LINK_GROUP:feature1,lib1,lib2>")

Be aware that the precedence of the group over the non-group link item can
result in circular dependencies between groups.  If this occurs, a fatal error
is raised because circular dependencies are not allowed for groups.

.. code-block:: cmake

  add_library(lib1A ...)
  add_library(lib1B ...)
  add_library(lib2A ...)
  add_library(lib2B ...)
  add_library(lib3 ...)

  # Non-group linking relationships, these are non-circular so far
  target_link_libraries(lib1A PUBLIC lib2A)
  target_link_libraries(lib2B PUBLIC lib1B)

  # The addition of these groups creates circular dependencies
  target_link_libraries(lib3 PRIVATE
    "$<LINK_GROUP:feat,lib1A,lib1B>"
    "$<LINK_GROUP:feat,lib2A,lib2B>"
  )

Because of the groups defined for ``lib3``, the linking relationships for
``lib1A`` and ``lib2B`` effectively get expanded to the equivalent of:

.. code-block:: cmake

  target_link_libraries(lib1A PUBLIC "$<LINK_GROUP:feat,lib2A,lib2B>")
  target_link_libraries(lib2B PUBLIC "$<LINK_GROUP:feat,lib1A,lib1B>")

This creates a circular dependency between groups: ``lib1A --> lib2B -->
lib1A``.

The following limitations should also be noted:

* The ``library-list`` can specify CMake targets or libraries.
  Any CMake target of type :ref:`OBJECT <Object Libraries>`
  or :ref:`INTERFACE <Interface Libraries>` will ignore the feature aspect
  of the expression and instead be linked in the standard way.

* The ``$<LINK_GROUP:...>`` generator expression can only be used to
  specify link libraries.  In practice, this means it can appear in the
  :prop_tgt:`LINK_LIBRARIES`, :prop_tgt:`INTERFACE_LINK_LIBRARIES`,and
  :prop_tgt:`INTERFACE_LINK_LIBRARIES_DIRECT` target properties, and be
  specified in :command:`target_link_libraries` and :command:`link_libraries`
  commands.

* If a ``$<LINK_GROUP:...>`` generator expression appears in the
  :prop_tgt:`INTERFACE_LINK_LIBRARIES` property of a target, it will be
  included in the imported target generated by a :command:`install(EXPORT)`
  command.  It is the responsibility of the environment consuming this
  import to define the link feature used by this expression.  Any CMake
  target named in the ``library-list`` should be wrapped in
  :genex:`TARGET_NAME` so that its name is namespaced on export; otherwise it
  is exported verbatim and will not resolve as an imported target.
