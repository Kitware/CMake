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
:ref:`ALIAS target <Alias Targets>`.  The ``<items>`` may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.

.. versionadded:: 3.20
  ``<target>`` can be a custom target.

The ``INTERFACE``, ``PUBLIC`` and ``PRIVATE`` keywords are required to
specify the scope of the source file paths (``<items>``) that follow
them.  ``PRIVATE`` and ``PUBLIC`` items will populate the :prop_tgt:`SOURCES`
property of ``<target>``, which are used when building the target itself.
``PUBLIC`` and ``INTERFACE`` items will populate the
:prop_tgt:`INTERFACE_SOURCES` property of ``<target>``, which are used
when building dependents.  A target created by :command:`add_custom_target`
can only have ``PRIVATE`` scope.

Repeated calls for the same ``<target>`` append items in the order called.

.. versionadded:: 3.3
  Allow exporting targets with :prop_tgt:`INTERFACE_SOURCES`.

.. versionadded:: 3.11
  Allow setting ``INTERFACE`` items on
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
  target_sources(MyTarget "$<$<CONFIG:Debug>:dbgsrc.cpp>")

  # CORRECT: absolute path used inside the generator expression
  target_sources(MyTarget "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/dbgsrc.cpp>")

See the :manual:`cmake-buildsystem(7)` manual for more on defining
buildsystem properties.

File Sets
^^^^^^^^^

.. versionadded:: 3.23

.. code-block:: cmake

  target_sources(<target>
    <INTERFACE|PUBLIC|PRIVATE> FILE_SET set1 [TYPE type1] [BASE_DIRS dirs1...] [FILES files1...]
    [<INTERFACE|PUBLIC|PRIVATE> FILE_SET set2 [TYPE type2] [BASE_DIRS dirs2...] [FILES files2...])

Adds a file set to a target, or adds files to an existing file set. Targets
have zero or more named file sets. Each file set has a name, a type, a scope of
``INTERFACE``, ``PUBLIC``, or ``PRIVATE``, one or more base directories, and
files within those directories. The only acceptable type is ``HEADERS``. The
optional default file sets are named after their type.

Files in a ``PRIVATE`` or ``PUBLIC`` file set are marked as source files for
the purposes of IDE integration. Additionally, files in ``HEADERS`` file sets
have their :prop_sf:`HEADER_FILE_ONLY` property set to ``TRUE``. Files in an
``INTERFACE`` or ``PUBLIC`` file set can be installed with the
:command:`install(TARGETS)` command, and exported with the
:command:`install(EXPORT)` and :command:`export` commands.

Each ``target_sources(FILE_SET)`` entry starts with ``INTERFACE``, ``PUBLIC``, or
``PRIVATE`` and accepts the following arguments:

``FILE_SET <set>``

  A string representing the name of the file set to create or add to. This must
  not start with a capital letter, unless its name is ``HEADERS``.

``TYPE <type>``

  A string representing the type of the file set. The only acceptable value is
  ``HEADERS``. This may be omitted if the name of the file set is ``HEADERS``.

``BASE_DIRS <dirs>``

  An optional list of strings representing the base directories of the file
  set. This argument supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`. No two
  ``BASE_DIRS`` may be sub-directories of each other. If no ``BASE_DIRS`` are
  specified when the file set is first created, the value of
  :variable:`CMAKE_CURRENT_SOURCE_DIR` is added.

``FILES <files>``

  An optional list of strings representing files in the file set. Each file
  must be in one of the ``BASE_DIRS``. This argument supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`. If relative
  paths are specified, they are considered relative to
  :variable:`CMAKE_CURRENT_SOURCE_DIR` at the time ``target_sources()`` is
  called, unless they start with ``$<``, in which case they are computed
  relative to the target's source directory after genex evaluation.

The following target properties are set by ``target_sources(FILE_SET)``:

:prop_tgt:`HEADER_SETS`

  List of ``PRIVATE`` and ``PUBLIC`` header sets associated with a target.
  Headers listed in these header sets are treated as source files for the
  purposes of IDE integration, and have their :prop_sf:`HEADER_FILE_ONLY`
  property set to ``TRUE``.

:prop_tgt:`INTERFACE_HEADER_SETS`

  List of ``INTERFACE`` and ``PUBLIC`` header sets associated with a target.
  Headers listed in these header sets can be installed with
  :command:`install(TARGETS)` and exported with :command:`install(EXPORT)` and
  :command:`export`.

:prop_tgt:`HEADER_SET`

  Headers in the default header set associated with a target. This property
  supports :manual:`generator expressions <cmake-generator-expressions(7)>`.

:prop_tgt:`HEADER_SET_<NAME>`

  Headers in the named header set ``<NAME>`` associated with a target. This
  property supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

:prop_tgt:`HEADER_DIRS`

  Base directories of the default header set associated with a target. This
  property supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

:prop_tgt:`HEADER_DIRS_<NAME>`

  Base directories of the header set ``<NAME>`` associated with a target. This
  property supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

:prop_tgt:`INCLUDE_DIRECTORIES`

  If the ``TYPE`` is ``HEADERS``, and the scope of the file set is ``PRIVATE``
  or ``PUBLIC``, all of the ``BASE_DIRS`` of the file set are wrapped in
  :genex:`$<BUILD_INTERFACE>` and appended to this property.

:prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES`

  If the ``TYPE`` is ``HEADERS``, and the scope of the file set is
  ``INTERFACE`` or ``PUBLIC``, all of the ``BASE_DIRS`` of the file set are
  wrapped in :genex:`$<BUILD_INTERFACE>` and appended to this property.
