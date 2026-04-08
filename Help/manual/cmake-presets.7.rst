.. cmake-manual-description: CMakePresets.json

cmake-presets(7)
****************

.. only:: html

   .. contents::

Introduction
============

.. versionadded:: 3.19

One problem that CMake users often face is sharing settings with other people
for common ways to configure a project. This may be done to support CI builds,
or for users who frequently use the same build. CMake supports two main files,
``CMakePresets.json`` and ``CMakeUserPresets.json``, that allow users to
specify common configure options and share them with others. CMake also
supports files included with the ``include`` field.

``CMakePresets.json`` and ``CMakeUserPresets.json`` live in the project's root
directory. They both have exactly the same format, and both are optional
(though at least one must be present if :cmake-option:`--preset`
is specified).  ``CMakePresets.json`` is meant to specify project-wide build
details, while ``CMakeUserPresets.json`` is meant for developers to specify
their own local build details.

``CMakePresets.json`` may be checked into a version control system, and
``CMakeUserPresets.json`` should NOT be checked in. For example, if a
project is using Git, ``CMakePresets.json`` may be tracked, and
``CMakeUserPresets.json`` should be added to the ``.gitignore``.

Format
======

The files are a JSON document with an object as the root:

.. literalinclude:: presets/example.json
  :language: json

.. presets-versionadded:: 10

  Preset files may include comments using the key ``$comment`` at any level
  within the JSON object to provide documentation.

The root object recognizes the following fields:

.. include:: presets/root-properties.rst

Includes
^^^^^^^^

``CMakePresets.json`` and ``CMakeUserPresets.json`` can include other files
with the ``include`` field in file version ``4`` and later. Files included
by these files can also include other files. If ``CMakePresets.json`` and
``CMakeUserPresets.json`` are both present, ``CMakeUserPresets.json``
implicitly includes ``CMakePresets.json``, even with no ``include`` field,
in all versions of the format.

If a preset file contains presets that inherit from presets in another file,
the file must include the other file either directly or indirectly.
Include cycles are not allowed among files. If ``a.json`` includes
``b.json``, ``b.json`` cannot include ``a.json``. However, a file may be
included multiple times from the same file or from different files.

Files directly or indirectly included from ``CMakePresets.json`` should be
guaranteed to be provided by the project. ``CMakeUserPresets.json`` may
include files from anywhere.

.. presets-versionchanged:: 7

  The ``include`` field supports `macro expansion`_, but only ``$penv{}`` macro
  expansion.

.. presets-versionchanged:: 9

  The ``include`` field supports `macro expansion`_, except for ``$env{}`` and
  preset-specific macros (i.e., those derived from the fields inside a preset's
  definition like ``presetName``).

.. _`Configure Preset`:

Configure Preset
^^^^^^^^^^^^^^^^

Each entry of the ``configurePresets`` array is a JSON object
that may contain the following fields:

.. include:: presets/configurePresets-properties.rst

.. _`Build Preset`:

Build Preset
^^^^^^^^^^^^

.. presets-versionadded:: 2

Each entry of the ``buildPresets`` array is a JSON object
that may contain the following fields:

.. include:: presets/buildPresets-properties.rst

.. _`Test Preset`:

Test Preset
^^^^^^^^^^^

.. presets-versionadded:: 2

Each entry of the ``testPresets`` array is a JSON object
that may contain the following fields:

.. include:: presets/testPresets-properties.rst

.. _`Package Preset`:

Package Preset
^^^^^^^^^^^^^^

.. presets-versionadded:: 6

Each entry of the ``packagePresets`` array is a JSON object
that may contain the following fields:

.. include:: presets/packagePresets-properties.rst

.. _`Workflow Preset`:

Workflow Preset
^^^^^^^^^^^^^^^

.. presets-versionadded:: 6

Each entry of the ``workflowPresets`` array is a JSON object
that may contain the following fields:

.. include:: presets/workflowPresets-properties.rst

Condition
^^^^^^^^^

.. presets-versionadded:: 3

The ``condition`` field of a preset is used to determine whether or not the
preset is enabled. For example, this can be used to disable a preset on
platforms other than Windows. ``condition`` may be either a boolean, ``null``,
or an object. If it is a boolean, the boolean indicates whether the preset is
enabled or disabled. If it is ``null``, the preset is enabled, but the ``null``
condition is not inherited by any presets that may inherit from the preset.
Sub-conditions (for example in a ``not``, ``anyOf``, or ``allOf`` condition)
may not be ``null``. If it is an object, it has the following fields:

``type``
  A required string with one of the following values:

  ``"const"``
    Indicates that the condition is constant. This is equivalent to using a
    boolean in place of the object. The condition object will have the
    following additional fields:

    ``value``
      A required boolean which provides a constant value for the condition's
      evaluation.

  ``"equals"``, ``"notEquals"``
    Indicates that the condition compares two strings to see if they are equal
    (or not equal). The condition object will have the following additional
    fields:

    ``lhs``
      First string to compare. This field supports macro expansion.

    ``rhs``
      Second string to compare. This field supports macro expansion.

  ``"inList"``, ``"notInList"``
    Indicates that the condition searches for a string in a list of strings.
    The condition object will have the following additional fields:

    ``string``
      A required string to search for. This field supports macro expansion.

    ``list``
      A required array of strings to search. This field supports macro
      expansion, and uses short-circuit evaluation.

  ``"matches"``, ``"notMatches"``
    Indicates that the condition searches for a regular expression in a string.
    The condition object will have the following additional fields:

    ``string``
      A required string to search. This field supports macro expansion.

    ``regex``
      A required regular expression to search for. This field supports macro
      expansion.

  ``"anyOf"``, ``"allOf"``

    Indicates that the condition is an aggregation of zero or more nested
    conditions. The condition object will have the following additional fields:

    ``conditions``
      A required array of condition objects. These conditions use short-circuit
      evaluation.

  ``"not"``
    Indicates that the condition is an inversion of another condition. The
    condition object will have the following additional fields:

    ``condition``
      A required condition object.

Macro Expansion
^^^^^^^^^^^^^^^

As mentioned above, some fields support macro expansion. Macros are
recognized in the form ``$<macro-namespace>{<macro-name>}``.

In general, macros are evaluated in the context of the preset being used, even
if the macro is in a field that was inherited from another preset. For example,
if the ``Base`` preset sets variable ``PRESET_NAME`` to ``${presetName}``, and
the ``Derived`` preset inherits from ``Base``, ``PRESET_NAME`` will be set to
``Derived``. The ``${fileDir}`` macro as of preset version ``12`` is an
exception to this rule.

It is an error to not put a closing brace at the end of a macro name. For
example, ``${sourceDir`` is invalid. A dollar sign (``$``) followed by
anything other than a left curly brace (``{``) with a possible namespace is
interpreted as a literal dollar sign.

Recognized macros include:

``${sourceDir}``
  Path to the project source directory (i.e. the same as
  :variable:`CMAKE_SOURCE_DIR`).

``${sourceParentDir}``
  Path to the project source directory's parent directory.

``${sourceDirName}``
  The last filename component of ``${sourceDir}``. For example, if
  ``${sourceDir}`` is ``/path/to/source``, this would be ``source``.

``${presetName}``
  Name specified in the preset's ``name`` field.

  This is a preset-specific macro.

``${generator}``
  Generator specified in the preset's ``generator`` field. For build and
  test presets, this will evaluate to the generator specified by
  ``configurePreset``.

  This is a preset-specific macro.

.. _`CMakePresets hostSystemName`:

``${hostSystemName}``
  .. presets-versionadded:: 3

  The name of the host operating system. Contains the same value as
  :variable:`CMAKE_HOST_SYSTEM_NAME`.

.. _`CMakePresets fileDir`:

``${fileDir}``
  .. presets-versionadded:: 4

  Path to the directory containing the preset file which defines the preset
  being used.

  .. presets-versionchanged:: 12

    This macro *always* expands to the directory of the current preset file
    containing the macro, regardless of the preset being used.

    For example, consider the following scenario.

    * ``/path/to/CMakePresets.json`` includes
      ``/path/to/subdir/CMakePresets.json``.
    * ``/path/to/subdir/CMakePresets.json`` defines preset ``Base``, which
      sets variable ``MY_DIR`` to ``${fileDir}``.
    * ``/path/to/CMakePresets.json`` defines preset ``Derived``, and
      ``Derived`` inherits from ``Base``.

    Under preset versions ``4``-``11``, ``MY_DIR`` will be set to ``/path/to/``
    when using the ``Base`` preset, and ``/path/to/subdir/`` when using the
    ``Derived`` preset.

    When ``/path/to/subdir/CMakePresets.json`` specifies version ``12`` or
    above, ``MY_DIR`` will always be set to ``/path/to/subdir/``, regardless of
    the preset being used.

    .. note::

      Since the ``${fileDir}`` macro in version 12 is expanded in the context
      of the current preset file, it is the version of the current file, rather
      than the version of the root file containing the preset being used, which
      enables this alternative behavior.

``${dollar}``
  A literal dollar sign (``$``).

.. _`CMakePresets pathListSep`:

``${pathListSep}``
  .. presets-versionadded:: 5

  Native character for separating lists of paths, such as ``:`` or ``;``.

  For example, by setting ``PATH`` to
  ``/path/to/ninja/bin${pathListSep}$env{PATH}``, ``${pathListSep}`` will
  expand to the underlying operating system's character used for
  concatenation in ``PATH``.

``$env{<variable-name>}``
  Environment variable with name ``<variable-name>``. The variable name may
  not be an empty string. If the variable is defined in the ``environment``
  field, that value is used instead of the value from the parent environment.
  If the environment variable is not defined, this evaluates as an empty
  string.

  Note that while Windows environment variable names are case-insensitive,
  variable names within a preset are still case-sensitive. This may lead to
  unexpected results when using inconsistent casing. For best results, keep
  the casing of environment variable names consistent.

``$penv{<variable-name>}``
  Similar to ``$env{<variable-name>}``, except that the value only comes from
  the parent environment, and never from the ``environment`` field. This
  allows one to prepend or append values to existing environment variables.
  For example, setting ``PATH`` to ``/path/to/ninja/bin:$penv{PATH}`` will
  prepend ``/path/to/ninja/bin`` to the ``PATH`` environment variable. This
  is needed because ``$env{<variable-name>}`` does not allow circular
  references.

``$vendor{<macro-name>}``
  An extension point for vendors to insert their own macros. CMake will not
  be able to use presets which have a ``$vendor{<macro-name>}`` macro, and
  effectively ignores such presets. However, it will still be able to use
  other presets from the same file.

  CMake does not make any attempt to interpret ``$vendor{<macro-name>}``
  macros. However, to avoid name collisions, IDE vendors should prefix
  ``<macro-name>`` with a very short (preferably <= 4 characters) vendor
  identifier prefix, followed by a ``.``, followed by the macro name. For
  example, the Example IDE could have ``$vendor{xide.ideInstallDir}``.

Versions
========

The JSON schema of ``CMakePresets.json`` and ``CMakeUserPresets.json``
follows a version scheme where new versions are added and allowed in newer
versions of CMake.

A list of the supported versions along with the version of CMake in which
they were added and a summary of the new features and changes is given below.

  ``1``
    .. versionadded:: 3.19

    The initial version supports `Configure Presets <Configure Preset_>`_
    and `Macro Expansion`_.

  ``2``
    .. versionadded:: 3.20

    * `Build Presets <Build Preset_>`_ were added.
    * `Test Presets <Test Preset_>`_ were added.

  ``3``
    .. versionadded:: 3.21

    * The `Condition`_ object was added for `Configure <Configure Preset_>`_,
      `Build <Build Preset_>`_, and `Test Presets <Test Preset_>`_.
    * Changes to `Configure Presets <Configure Preset_>`_

      * The `installDir <CMakePresets.configurePresets.installDir_>`_ field was
        added.
      * The `toolchainFile <CMakePresets.configurePresets.toolchainFile_>`_
        field was added.
      * The `binaryDir <CMakePresets.configurePresets.binaryDir_>`_ field is
        now optional.
      * The `generator <CMakePresets.configurePresets.generator_>`_ field is
        now optional.

    * Changes to `Macro Expansion`_

      * The `${hostSystemName} <CMakePresets hostSystemName_>`_ macro was
        added.

  ``4``
    .. versionadded:: 3.23

    * `Includes`_ were added to support including other JSON files in
      ``CMakePresets.json`` and ``CMakeUserPresets.json``.
    * Changes to `Build Presets <Build Preset_>`_

      * The
        `resolvePackageReferences <CMakePresets.buildPresets.resolvePackageReferences_>`_
        field was added.

    * Changes to `Macro Expansion`_

      * The `${fileDir} <CMakePresets fileDir_>`_ macro was added.

  ``5``
    .. versionadded:: 3.24

    * Changes to `Test Presets <Test Preset_>`_

      * The `testOutputTruncation <CMakePresets.testPresets.output.testOutputTruncation_>`_
        field was added to the `output <CMakePresets.testPresets.output_>`_
        object.

    * Changes to `Macro Expansion`_

      * The `${pathListSep} <CMakePresets pathListSep_>`_ macro was added.

  ``6``
    .. versionadded:: 3.25

    * `Package Presets <Package Preset_>`_ were added.
    * `Workflow Presets <Workflow Preset_>`_ were added.
    * Changes to `Test Presets <Test Preset_>`_

      * The `outputJUnitFile <CMakePresets.testPresets.output.outputJUnitFile_>`_
        field was added to the `output <CMakePresets.testPresets.output_>`_
        object.

  ``7``
    .. versionadded:: 3.27

    * Changes to `Configure Presets <Configure Preset_>`_

      * The `trace <CMakePresets.configurePresets.trace_>`_ field was added.

    * Changes to `Includes`_

      * The ``include`` field now supports ``$penv{}`` `macro expansion`_.

  ``8``
    .. versionadded:: 3.28

    * The `$schema <CMakePresets.$schema_>`_ field was added to the root
      object.

  ``9``
    .. versionadded:: 3.30

    * Changes to `Includes`_

      * The ``include`` field now supports other types of `macro expansion`_.

  ``10``
    .. versionadded:: 3.31

    * The optional ``$comment`` field was added to support documentation
      throughout ``CMakePresets.json`` and ``CMakeUserPresets.json``.
    * Changes to `Configure Presets <Configure Preset_>`_:

      * The `graphviz <CMakePresets.configurePresets.graphviz_>`_ field was
        added.

  ``11``
    .. versionadded:: 4.3

    * Changes to `Test Presets <Test Preset_>`_

      * The `jobs <CMakePresets.testPresets.execution.jobs_>`_ field now
        accepts an empty string representing
        :ctest-option:`--parallel` with ``<jobs>`` omitted.

  ``12``
    .. versionadded:: 4.4

    * Changes to `Macro Expansion`_

      * The `${fileDir} <CMakePresets fileDir_>`_ macro now always expands to
        the directory of preset file containing the ``${fileDir}`` macro,
        regardless of whether it is inherited by another preset in a different
        directory.

    * Changes to `Test Presets <Test Preset_>`_

      * The `testPassthroughArguments <CMakePresets.testPresets.execution.testPassthroughArguments_>`_
        field was added to forward arguments to test executables.

Schema
======

:download:`This file </manual/presets/schema.json>` provides a machine-readable
JSON schema for the ``CMakePresets.json`` format.
