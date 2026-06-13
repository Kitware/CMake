source_group
------------

Define a grouping for source files in IDE project generation.
There are two different signatures to create source groups.

Synopsis
^^^^^^^^

.. parsed-literal::

  source_group(`\<group\>`_ [FILES <src>...] [REGULAR_EXPRESSION <regex>]
                       [FILE_SETS <fileset>... TARGET <target>]...)

  source_group(`TREE`_ <root> [PREFIX <prefix>] [FILES <src>...])

Overview
^^^^^^^^

Defines a group into which sources will be placed in project files.
This is intended to set up file tabs in Visual Studio.

The group is scoped in the directory where the command is called, and applies
to sources in targets created in that directory.

If the file is part of a file set, search for a group which explicitly lists
this file set. If no group has been found or the file is not part of a file
set, search for a group which explicitly lists the file.

If the file set or the source file matches multiple groups, the *last* group
that explicitly lists the file set with ``FILE_SETS`` or the file with
``FILES`` will be favored, if any.

If no group explicitly lists the file set or the file, the *last* group whose
regular expression matches the file will be favored.

The ``<group>`` and ``<prefix>`` arguments may contain forward
slashes or backslashes to specify subgroups.  Backslashes need to be escaped
appropriately:

.. code-block:: cmake

  source_group(base/subdir ...)
  source_group(outer\\inner ...)
  source_group(TREE <root> PREFIX sources\\inc ...)

.. versionadded:: 3.18
  Allow using forward slashes (``/``) to specify subgroups.

Commands
^^^^^^^^

.. signature::
  source_group(<group> [FILES <src>...] [REGULAR_EXPRESSION <regex>]
                       [FILE_SETS <fileset>... TARGET <target>]...)
  :target: <group>

  The options are:

  ``<group>``
    The name of the group.

  ``FILES <src>...``
    Any source file specified explicitly will be placed in group
    ``<group>``.  Relative paths are interpreted with respect to the
    current source directory.

    .. versionadded:: 4.3
      Arguments to ``FILES`` may use
      :manual:`generator expressions <cmake-generator-expressions(7)>`.

  ``REGULAR_EXPRESSION <regex>``
    Any source file whose name matches the regular expression will
    be placed in group ``<group>``.

  ``FILE_SETS <fileset>...``
    .. versionadded:: 4.5

    List of file sets. Files of these file sets will be placed in group
    ``<group>``. Arguments to ``FILE_SETS`` may use
    :manual:`generator expressions <cmake-generator-expressions(7)>`.

  ``TARGET <target>``
    .. versionadded:: 4.5

    Associate the file sets declared with the previous ``FILE_SETS`` keyword
    with the target ``<target>``.

  .. note::

    The pattern ``FILE_SETS <fileset>... TARGET <target>`` can be repeated
    multiple times.

.. signature::
  source_group(TREE <root> [PREFIX <prefix>] [FILES <src>...])

  .. versionadded:: 3.8

  The options are:

  ``TREE <root>``
    CMake will automatically detect, from ``<src>`` files paths, source groups
    it needs to create, to keep structure of source groups analogically to the
    actual files and directories structure in the project. Paths of ``<src>``
    files will be cut to be relative to ``<root>``. The command fails if the
    paths within ``src`` do not start with ``root``.

  ``PREFIX <prefix>``
    Source group and files located directly in ``<root>`` path, will be placed
    in ``<prefix>`` source groups.

  ``FILES <src>...``
    List of files used to create source group structure. Relative paths are
    interpreted with respect to the current source directory.

    .. versionadded:: 4.3
      Arguments to ``FILES`` may use
      :manual:`generator expressions <cmake-generator-expressions(7)>`.

Legacy Support
^^^^^^^^^^^^^^
For backwards compatibility, the short-hand signature

.. code-block:: cmake

  source_group(<name> <regex>)

is equivalent to

.. code-block:: cmake

  source_group(<name> REGULAR_EXPRESSION <regex>)
