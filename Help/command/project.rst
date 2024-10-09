project
-------

Set the name of the project.

Synopsis
^^^^^^^^

.. code-block:: cmake

 project(<PROJECT-NAME> [<language-name>...])
 project(<PROJECT-NAME>
         [VERSION <major>[.<minor>[.<patch>[.<tweak>]]]]
         [DESCRIPTION <project-description-string>]
         [HOMEPAGE_URL <url-string>]
         [LANGUAGES <language-name>...])

Sets the name of the project, and stores it in the variable
:variable:`PROJECT_NAME`. When called from the top-level
``CMakeLists.txt`` also stores the project name in the
variable :variable:`CMAKE_PROJECT_NAME`.

Also sets the variables:

:variable:`PROJECT_SOURCE_DIR`, :variable:`<PROJECT-NAME>_SOURCE_DIR`
  Absolute path to the source directory for the project.

:variable:`PROJECT_BINARY_DIR`, :variable:`<PROJECT-NAME>_BINARY_DIR`
  Absolute path to the binary directory for the project.

:variable:`PROJECT_IS_TOP_LEVEL`, :variable:`<PROJECT-NAME>_IS_TOP_LEVEL`
  .. versionadded:: 3.21

  Boolean value indicating whether the project is top-level.

Further variables are set by the optional arguments described in `Options`_
further below. Where an option is not given, its corresponding variable is
set to the empty string.

Note that variables of the form ``<name>_SOURCE_DIR`` and ``<name>_BINARY_DIR``
may also be set by other commands before ``project()`` is called (see the
:command:`FetchContent_MakeAvailable` command for one example).
Projects should not rely on ``<PROJECT-NAME>_SOURCE_DIR`` or
``<PROJECT-NAME>_BINARY_DIR`` holding a particular value outside of the scope
of the call to ``project()`` or one of its child scopes.

.. versionchanged:: 3.30
  ``<PROJECT-NAME>_SOURCE_DIR``, ``<PROJECT-NAME>_BINARY_DIR``, and
  ``<PROJECT-NAME>_IS_TOP_LEVEL``, if already set as normal variables when
  ``project(<PROJECT-NAME> ...)`` is called, are updated by the call.
  Cache entries by the same names are always set as before.
  See release notes for 3.30.3, 3.30.4, and 3.30.5 for details.

.. versionchanged:: 3.31
  ``<PROJECT-NAME>_SOURCE_DIR``, ``<PROJECT-NAME>_BINARY_DIR``, and
  ``<PROJECT-NAME>_IS_TOP_LEVEL`` are always set as normal variables by
  ``project(<PROJECT-NAME> ...)``.  See policy :policy:`CMP0180`.
  Cache entries by the same names are always set as before.

Options
^^^^^^^

The options are:

``VERSION <version>``
  Optional; may not be used unless policy :policy:`CMP0048` is
  set to ``NEW``.

  Takes a ``<version>`` argument composed of non-negative integer components,
  i.e. ``<major>[.<minor>[.<patch>[.<tweak>]]]``,
  and sets the variables

  * :variable:`PROJECT_VERSION`,
    :variable:`<PROJECT-NAME>_VERSION`
  * :variable:`PROJECT_VERSION_MAJOR`,
    :variable:`<PROJECT-NAME>_VERSION_MAJOR`
  * :variable:`PROJECT_VERSION_MINOR`,
    :variable:`<PROJECT-NAME>_VERSION_MINOR`
  * :variable:`PROJECT_VERSION_PATCH`,
    :variable:`<PROJECT-NAME>_VERSION_PATCH`
  * :variable:`PROJECT_VERSION_TWEAK`,
    :variable:`<PROJECT-NAME>_VERSION_TWEAK`.

  .. versionadded:: 3.12
    When the ``project()`` command is called from the top-level
    ``CMakeLists.txt``, then the version is also stored in the variable
    :variable:`CMAKE_PROJECT_VERSION`.

``DESCRIPTION <project-description-string>``
  .. versionadded:: 3.9

  Optional.
  Sets the variables

  * :variable:`PROJECT_DESCRIPTION`, :variable:`<PROJECT-NAME>_DESCRIPTION`

  to ``<project-description-string>``.
  It is recommended that this description is a relatively short string,
  usually no more than a few words.

  When the ``project()`` command is called from the top-level ``CMakeLists.txt``,
  then the description is also stored in the variable :variable:`CMAKE_PROJECT_DESCRIPTION`.

  .. versionadded:: 3.12
    Added the ``<PROJECT-NAME>_DESCRIPTION`` variable.

``HOMEPAGE_URL <url-string>``
  .. versionadded:: 3.12

  Optional.
  Sets the variables

  * :variable:`PROJECT_HOMEPAGE_URL`, :variable:`<PROJECT-NAME>_HOMEPAGE_URL`

  to ``<url-string>``, which should be the canonical home URL for the project.

  When the ``project()`` command is called from the top-level ``CMakeLists.txt``,
  then the URL also is stored in the variable :variable:`CMAKE_PROJECT_HOMEPAGE_URL`.

``LANGUAGES <language-name>...``
  Optional.
  Can also be specified without ``LANGUAGES`` keyword per the first, short signature.

  Selects which programming languages are needed to build the project.

.. include:: SUPPORTED_LANGUAGES.txt

By default ``C`` and ``CXX`` are enabled if no language options are given.
Specify language ``NONE``, or use the ``LANGUAGES`` keyword and list no languages,
to skip enabling any languages.

The variables set through the ``VERSION``, ``DESCRIPTION`` and ``HOMEPAGE_URL``
options are intended for use as default values in package metadata and documentation.

.. _`Code Injection`:

Code Injection
^^^^^^^^^^^^^^

A number of variables can be defined by the user to specify files to include
at different points during the execution of the ``project()`` command.
The following outlines the steps performed during a ``project()`` call:

* .. versionadded:: 3.15
    For every ``project()`` call regardless of the project
    name, include the file(s) and module(s) named by
    :variable:`CMAKE_PROJECT_INCLUDE_BEFORE`, if set.

* .. versionadded:: 3.17
    If the ``project()`` command specifies ``<PROJECT-NAME>`` as its project
    name, include the file(s) and module(s) named by
    :variable:`CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE_BEFORE`, if set.

* Set the various project-specific variables detailed in the `Synopsis`_
  and `Options`_ sections above.

* For the very first ``project()`` call only:

  * If :variable:`CMAKE_TOOLCHAIN_FILE` is set, read it at least once.
    It may be read multiple times and it may also be read again when
    enabling languages later (see below).

  * Set the variables describing the host and target platforms.
    Language-specific variables might or might not be set at this point.
    On the first run, the only language-specific variables that might be
    defined are those a toolchain file may have set. On subsequent runs,
    language-specific variables cached from a previous run may be set.

  * .. versionadded:: 3.24
      Include each file listed in :variable:`CMAKE_PROJECT_TOP_LEVEL_INCLUDES`,
      if set. The variable is ignored by CMake thereafter.

* Enable any languages specified in the call, or the default languages if
  none were provided. The toolchain file may be re-read when enabling a
  language for the first time.

* .. versionadded:: 3.15
    For every ``project()`` call regardless of the project
    name, include the file(s) and module(s) named by
    :variable:`CMAKE_PROJECT_INCLUDE`, if set.

* If the ``project()`` command specifies ``<PROJECT-NAME>`` as its project
  name, include the file(s) and module(s) named by
  :variable:`CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE`, if set.

Usage
^^^^^

The top-level ``CMakeLists.txt`` file for a project must contain a
literal, direct call to the ``project()`` command; loading one
through the :command:`include` command is not sufficient.  If no such
call exists, CMake will issue a warning and pretend there is a
``project(Project)`` at the top to enable the default languages
(``C`` and ``CXX``).

.. note::
  Call the ``project()`` command near the top of the top-level
  ``CMakeLists.txt``, but *after* calling :command:`cmake_minimum_required`.
  It is important to establish version and policy settings before invoking
  other commands whose behavior they may affect and for this reason the
  ``project()`` command will issue a warning if this order is not kept.
  See also policy :policy:`CMP0000`.
