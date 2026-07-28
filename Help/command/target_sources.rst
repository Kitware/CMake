target_sources
--------------

.. versionadded:: 3.1

Add sources to a target.

.. code-block:: cmake

  target_sources(<target>
    {INTERFACE|PUBLIC|PRIVATE} <source>...
    [{INTERFACE|PUBLIC|PRIVATE} <source>...]...)

Specifies sources to use when building a target and/or its dependents.
The named ``<target>`` must have been created by a command such as
:command:`add_executable` or :command:`add_library` or
:command:`add_custom_target`.  The ``<source>`` items may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.

.. versionadded:: 3.20
  ``<target>`` can be a custom target.

.. versionchanged:: 4.5
  If ``<target>`` is an :ref:`Alias Target <Alias Targets>`, the command
  operates on the target which the alias references.

The ``INTERFACE``, ``PUBLIC`` and ``PRIVATE`` keywords are required to
specify the :ref:`scope <Target Command Scope>` of the source file paths
(``<source>...``) that follow them.  ``PRIVATE`` and ``PUBLIC`` sources will
populate the :prop_tgt:`SOURCES` property of ``<target>``, which are used when
building the target itself. ``PUBLIC`` and ``INTERFACE`` sources will populate
the :prop_tgt:`INTERFACE_SOURCES` property of ``<target>``, which are used
when building dependents.  A target created by :command:`add_custom_target`
can only have ``PRIVATE`` scope.

Repeated calls for the same ``<target>`` append sources in the order called.

.. versionadded:: 3.3
  Allow exporting targets with :prop_tgt:`INTERFACE_SOURCES`.

.. versionadded:: 3.11
  Allow setting ``INTERFACE`` sources on
  :ref:`IMPORTED targets <Imported Targets>`.

.. versionchanged:: 3.13
  Relative source file paths are interpreted as being relative to the current
  source directory (i.e. :variable:`CMAKE_CURRENT_SOURCE_DIR`).
  See policy :policy:`CMP0076`.

A path that begins with a generator expression is left unmodified.
When a target's :prop_tgt:`SOURCE_DIR` property differs from
:variable:`CMAKE_CURRENT_SOURCE_DIR`, use absolute paths in generator
expressions to ensure the sources are correctly assigned to the target.

.. code-block:: cmake

  # WRONG: starts with generator expression, but relative path used
  target_sources(MyTarget PRIVATE "$<$<CONFIG:Debug>:dbgsrc.cpp>")

  # CORRECT: absolute path used inside the generator expression
  target_sources(MyTarget PRIVATE "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/dbgsrc.cpp>")

See the :manual:`cmake-buildsystem(7)` manual for more on defining
buildsystem properties.

File Sets
^^^^^^^^^

.. versionadded:: 3.23

.. code-block:: cmake

  target_sources(<target>
    [{INTERFACE|PUBLIC|PRIVATE}
     [FILE_SET <set> [TYPE <type>] [BASE_DIRS <dirs>...] [FILES <files>...]]...
    ]...)

Adds a file set to a target, or adds files to an existing file set. Targets
have zero or more named file sets.

See the :manual:`cmake-buildsystem(7)` manual for more on :ref:`File Sets`,
including their types and scope semantics, properties, and installation/export
behavior.

Each ``target_sources(FILE_SET)`` entry starts with ``INTERFACE``, ``PUBLIC``,
or ``PRIVATE`` and accepts the following arguments:

``FILE_SET <set>``

  The name of the file set to create or add to. It must contain only letters,
  numbers and underscores. Names starting with a capital letter are reserved
  for built-in file sets predefined by CMake. The only predefined set names
  are those matching the acceptable types. All other set names must not start
  with a capital letter or
  underscore.

``TYPE <type>``

  Every file set is associated with a particular type of file. Only types
  listed under :ref:`File Sets` may be used and it is an error to specify
  anything else. As a special case, if the name of the file set is one of these
  types, the type does not need to be specified and the ``TYPE <type>``
  arguments can be omitted. For all other file set names, ``TYPE`` is required.

``BASE_DIRS <dirs>...``

  An optional list of base directories of the file set. Any relative path
  is treated as relative to the current source directory
  (i.e. :variable:`CMAKE_CURRENT_SOURCE_DIR`). If no ``BASE_DIRS`` are
  specified when the file set is first created, the value of
  :variable:`CMAKE_CURRENT_SOURCE_DIR` is added. This argument supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

  No two base directories for a file set may be sub-directories of each other.
  This requirement must be met across all base directories added to a file set,
  not just those within a single call to ``target_sources()``.

``FILES <files>...``

  An optional list of files to add to the file set. Each file must be in
  one of the base directories, or a subdirectory of one of the base
  directories. This argument supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

  If relative paths are specified, they are considered relative to
  :variable:`CMAKE_CURRENT_SOURCE_DIR` at the time ``target_sources()`` is
  called. An exception to this is a path starting with ``$<``. Such paths
  are treated as relative to the target's source directory after evaluation
  of generator expressions.

See Also
^^^^^^^^

* :command:`add_executable`
* :command:`add_library`
* :command:`target_compile_definitions`
* :command:`target_compile_features`
* :command:`target_compile_options`
* :command:`target_include_directories`
* :command:`target_link_libraries`
* :command:`target_link_directories`
* :command:`target_link_options`
* :command:`target_precompile_headers`
