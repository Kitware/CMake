target_sources
--------------

.. versionadded:: 3.1

Add sources to a target.

.. code-block:: cmake

  target_sources(<target>
    <INTERFACE|PUBLIC|PRIVATE> [items1...]
    [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])

Specifies sources to use when building a target and/or its dependents.
The named ``<target>`` must have been created by a command such as
:command:`add_executable` or :command:`add_library` or
:command:`add_custom_target` and must not be an
:ref:`ALIAS target <Alias Targets>`.

.. versionchanged:: 3.13
  Relative source file paths are interpreted as being relative to the current
  source directory (i.e. :variable:`CMAKE_CURRENT_SOURCE_DIR`).
  See policy :policy:`CMP0076`.

.. versionadded:: 3.20
  ``<target>`` can be a custom target.

The ``INTERFACE``, ``PUBLIC`` and ``PRIVATE`` keywords are required to
specify the scope of the items following them.  ``PRIVATE`` and ``PUBLIC``
items will populate the :prop_tgt:`SOURCES` property of
``<target>``, which are used when building the target itself.
``PUBLIC`` and ``INTERFACE`` items will populate the
:prop_tgt:`INTERFACE_SOURCES` property of ``<target>``, which are used
when building dependents.
The following arguments specify sources.  Repeated calls for the same
``<target>`` append items in the order called. The targets created by
:command:`add_custom_target` can only have ``PRIVATE`` scope.

.. versionadded:: 3.3
  Allow exporting targets with :prop_tgt:`INTERFACE_SOURCES`.

.. versionadded:: 3.11
  Allow setting ``INTERFACE`` items on :ref:`IMPORTED targets <Imported Targets>`.

Arguments to ``target_sources`` may use "generator expressions"
with the syntax ``$<...>``. See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  See the :manual:`cmake-buildsystem(7)`
manual for more on defining buildsystem properties.
