cmake_path
----------

.. versionadded:: 3.19

Filesystem path manipulation command.

This command is dedicated to the manipulation of objects of type path which
represent paths on a filesystem. Only syntactic aspects of paths are handled:
the pathname may represent a non-existing path or even one that is not allowed
to exist on the current file system or OS.

For operations involving the filesystem, have a look at the :command:`file`
command.

The path name has the following syntax:

1. ``root-name`` (optional): identifies the root on a filesystem with multiple
   roots (such as ``"C:"`` or ``"//myserver"``).

2. ``root-directory`` (optional): a directory separator that, if present, marks
   this path as absolute. If it is missing (and the first element other than
   the ``root-name`` is a ``item-name``), then the path is relative.

Zero or more of the following:

3. ``item-name``: sequence of characters that aren't directory separators. This
   name may identify a file, a hard link, a symbolic link, or a directory. Two
   special ``item-names`` are recognized:

     * ``dot``: the item name consisting of a single dot character ``.`` is a
       directory name that refers to the current directory.

     * ``dot-dot``: the item name consisting of two dot characters ``..`` is a
       directory name that refers to the parent directory.

4. ``directory-separator``: the forward slash character ``/``. If this
   character is repeated, it is treated as a single directory separator:
   ``/usr///////lib`` is the same as ``/usr/lib``.

.. _FILENAME_DEF:

A path has a filename if it does not ends with a ``directory-separator``. The
filename is the last ``item-name`` of the path.

.. _EXTENSION_DEF:

A :ref:`filename <FILENAME_DEF>` can have an extension. By default, the
extension is defined as the sub-string beginning at the leftmost period
(including the period) and until the end of the pathname. When the option
``LAST_ONLY`` is specified, the extension is the sub-string beginning at the
rightmost period.

The following exceptions apply:

  * If the first character in the :ref:`filename <FILENAME_DEF>` is a period,
    that period is ignored (a filename like ``".profile"`` is not treated as an
    extension).

  * If the pathname is either ``.`` or ``..``.

.. note::

  ``cmake_path`` command handles paths in the format of the build system, not
  the target system. So this is not generally applicable to the target system
  in cross-compiling environment.

For all commands, ``<path-var>`` placeholder expect a variable name. An error
will be raised if the variable does not exist, except for `APPEND`_ and
`CMAKE_PATH`_ sub-commands. ``<input>`` placeholder expect a string literal.
``[<input>...]`` placeholder expect zero or more arguments. ``<out-var>``
placeholder expect a variable name.

.. note::

  ``cmake_path`` command does not support list of paths. The ``<path-var>``
  placeholder must store only one path name.

To initialize a path variable, three possibilities can be used:

1. :command:`set` command.
2. :ref:`cmake_path(APPEND) <APPEND>` command. Can be used to build a path from
   already available path fragments.
3. :ref:`cmake_path(CMAKE_PATH) <CMAKE_PATH>` command. Mainly used to build a
   path variable from a native path.

  .. code-block:: cmake

    # To build the path "${CMAKE_CURRENT_SOURCE_DIR}/data"

    set (path1 "${CMAKE_CURRENT_SOURCE_DIR}/data")

    cmake_path(APPEND path2 "${CMAKE_CURRENT_SOURCE_DIR}" "data")

    cmake_path(CMAKE_PATH path3 "${CMAKE_CURRENT_SOURCE_DIR}/data")

`Modification`_ and `Generation`_ sub-commands store the result in-place or in
the variable specified by  ``OUTPUT_VARIABLE`` option. All other sub-commands,
except `CMAKE_PATH`_, store the result in the required ``<out-var>`` variable.

Sub-commands supporting ``NORMALIZE`` option will :ref:`normalize <NORMAL_PATH>`
the path.

Synopsis
^^^^^^^^

.. parsed-literal::

  `Decomposition`_
    cmake_path(`GET`_ <path-var> :ref:`ROOT_NAME <GET_ROOT_NAME>` <out-var>)
    cmake_path(`GET`_ <path-var> :ref:`ROOT_DIRECTORY <GET_ROOT_DIRECTORY>` <out-var>)
    cmake_path(`GET`_ <path-var> :ref:`ROOT_PATH <GET_ROOT_PATH>` <out-var>)
    cmake_path(`GET`_ <path-var> :ref:`FILENAME <GET_FILENAME>` <out-var>)
    cmake_path(`GET`_ <path-var> :ref:`EXTENSION <GET_EXTENSION>` [LAST_ONLY] <out-var>)
    cmake_path(`GET`_ <path-var> :ref:`STEM <GET_STEM>` [LAST_ONLY] <out-var>)
    cmake_path(`GET`_ <path-var> :ref:`RELATIVE_PATH <GET_RELATIVE_PATH>` <out-var>)
    cmake_path(`GET`_ <path-var> :ref:`PARENT_PATH <GET_PARENT_PATH>` <out-var>)

  `Modification`_
    cmake_path(`APPEND`_ <path-var> [<input>...] [OUTPUT_VARIABLE <out-var>])
    cmake_path(`CONCAT`_ <path-var> [<input>...] [OUTPUT_VARIABLE <out-var>])
    cmake_path(`REMOVE_FILENAME`_ <path-var> [OUTPUT_VARIABLE <out-var>])
    cmake_path(`REPLACE_FILENAME`_ <path-var> <input> [OUTPUT_VARIABLE <out-var>])
    cmake_path(`REMOVE_EXTENSION`_ <path-var> [LAST_ONLY]
                                              [OUTPUT_VARIABLE <out-var>])
    cmake_path(`REPLACE_EXTENSION`_ <path-var> [LAST_ONLY] <input>
                                               [OUTPUT_VARIABLE <out-var>])

  `Generation`_
    cmake_path(`NORMAL_PATH`_ <path-var> [OUTPUT_VARIABLE <out-var>])
    cmake_path(`RELATIVE_PATH`_ <path-var> [BASE_DIRECTORY <input>]
                                           [OUTPUT_VARIABLE <out-var>])
    cmake_path(`PROXIMATE_PATH`_ <path-var> [BASE_DIRECTORY <input>]
                                            [OUTPUT_VARIABLE <out-var>])
    cmake_path(`ABSOLUTE_PATH`_ <path-var> [BASE_DIRECTORY <input>] [NORMALIZE]
                                           [OUTPUT_VARIABLE <out-var>])

  `Conversion`_
    cmake_path(`CMAKE_PATH`_ <path-var> [NORMALIZE] <input>)
    cmake_path(`NATIVE_PATH`_ <path-var> [NORMALIZE] <out-var>)
    cmake_path(`CONVERT`_ <input> `TO_CMAKE_PATH_LIST`_ <out-var>)
    cmake_path(`CONVERT`_ <input> `TO_NATIVE_PATH_LIST`_ <out-var>)

  `Comparison`_
    cmake_path(`COMPARE`_ <path-var> <OP> <input> <out-var>)

  `Query`_
    cmake_path(`HAS_ROOT_NAME`_ <path-var> <out-var>)
    cmake_path(`HAS_ROOT_DIRECTORY`_ <path-var> <out-var>)
    cmake_path(`HAS_ROOT_PATH`_ <path-var> <out-var>)
    cmake_path(`HAS_FILENAME`_ <path-var> <out-var>)
    cmake_path(`HAS_EXTENSION`_ <path-var> <out-var>)
    cmake_path(`HAS_STEM`_ <path-var> <out-var>)
    cmake_path(`HAS_RELATIVE_PATH`_ <path-var> <out-var>)
    cmake_path(`HAS_PARENT_PATH`_ <path-var> <out-var>)
    cmake_path(`IS_ABSOLUTE`_ <path-var> <out-var>)
    cmake_path(`IS_RELATIVE`_ <path-var> <out-var>)
    cmake_path(`IS_PREFIX`_ <path-var> <input> [NORMALIZE] <out-var>)

  `Hashing`_
    cmake_path(`HASH`_ <path-var> [NORMALIZE] <out-var>)

Decomposition
^^^^^^^^^^^^^

.. _GET:
.. _GET_ROOT_NAME:

.. code-block:: cmake

  cmake_path(GET <path-var> ROOT_NAME <out-var>)

Returns the root name of the path. If the path does not include a root name,
returns an empty path.

.. note::

  Only ``Windows`` system has the concept of ``root-name``, so on all other
  systems, it is always an empty path.

For example:

  .. code-block:: cmake

    set (path "c:/a")
    cmake_path (GET path ROOT_NAME output)
    message ("Root name is \"${output}\"")

  Will display::

    Root name is "c:"

.. _GET_ROOT_DIRECTORY:

.. code-block:: cmake

  cmake_path(GET <path-var> ROOT_DIRECTORY <out-var>)

Returns the root directory of the path. If the path does not include a root
directory, returns an empty path.

For example:

  .. code-block:: cmake

    set (path "c:/a")
    cmake_path (GET path ROOT_DIRECTORY output)
    message ("Root directory is \"${output}\"")

  Will display::

    Root directory is "/"

.. _GET_ROOT_PATH:

.. code-block:: cmake

  cmake_path(GET <path-var> ROOT_PATH <out-var>)

Returns the root path of the path. If the path does not include a root path,
returns an empty path.

Effectively, returns the following: ``root-name root-directory``.

For example:

  .. code-block:: cmake

    set (path "c:/a")
    cmake_path (GET path ROOT_PATH output)
    message ("Root path is \"${output}\"")

  Will display::

    Root path is "c:/"

.. _GET_FILENAME:

.. code-block:: cmake

  cmake_path(GET <path-var> FILENAME <out-var>)

Returns the :ref:`filename <FILENAME_DEF>` component of the path. If the path
ends with a ``directory-separator``, there is no filename, so returns an empty
path.

For example:

  .. code-block:: cmake

    set (path "/a")
    cmake_path (GET path FILENAME output)
    message ("First filename is \"${output}\"")

    set (path "/a/")
    cmake_path (GET path FILENAME output)
    message ("Second filename is \"${output}\"")

  Will display::

    First filename is "a"
    Second filename is ""

.. _GET_EXTENSION:

.. code-block:: cmake

  cmake_path(GET <path-var> EXTENSION [LAST_ONLY] <out-var>)

Returns the :ref:`extension <EXTENSION_DEF>` of the filename component.

If the :ref:`filename <FILENAME_DEF>` component of the path contains a period
(``.``), and is not one of the special filesystem elements ``dot`` or
``dot-dot``, then the :ref:`extension <EXTENSION_DEF>` is returned.

For example:

  .. code-block:: cmake

    set (path "name.ext1.ext2")
    cmake_path (GET path EXTENSION result)
    message ("Full extension is \"${result}\"")
    cmake_path (GET path EXTENSION LAST_ONLY result)
    message ("Last extension is \"${result}\"")

  Will display::

    Full extension is ".ext1.ext2"
    Last extension is ".ext2"

The following exceptions apply:

  * If the first character in the filename is a period, that period is ignored
    (a filename like ``".profile"`` is not treated as an extension).

  * If the pathname is either ``.`` or ``..``, or if
    :ref:`filename <FILENAME_DEF>` component does not contain the ``.``
    character, then an empty path is returned.

.. _GET_STEM:

.. code-block:: cmake

  cmake_path(GET <path-var> STEM [LAST_ONLY] <out-var>)

Returns the :ref:`filename <FILENAME_DEF>` component of the path stripped of
its :ref:`extension <EXTENSION_DEF>`.

For Example:

  .. code-block:: cmake

    set (path "name.ext1.ext2")
    cmake_path (GET path STEM result)
    message ("Filename without the extension is \"${result}\"")
    cmake_path (GET path STEM LAST_ONLY result)
    message ("Filename whiteout the last extension is \"${result}\"")

  Will display::

    Filename without the extension is "name"
    Filename without the last extension is "name.ext1"

The following exceptions apply:

  * If the first character in the filename is a period, that period is ignored
    (a filename like ``".profile"`` is not treated as an extension).

  * If the filename is one of the special filesystem components ``dot`` or
    ``dot-dot``, or if it has no periods, the function returns the entire
    :ref:`filename <FILENAME_DEF>` component.

.. _GET_RELATIVE_PATH:

.. code-block:: cmake

  cmake_path(GET <path-var> RELATIVE_PATH <out-var>)

Returns path relative to ``root-path``, that is, a pathname composed of
every component of ``<path-var>`` after ``root-path``. If ``<path-var>`` is
an empty path, returns an empty path.

For Example:

  .. code-block:: cmake

    set (path "/a/b")
    cmake_path (GET path RELATIVE_PATH result)
    message ("Relative path is \"${result}\"")

    set (path "/")
    cmake_path (GET path RELATIVE_PATH result)
    message ("Relative path is \"${result}\"")

  Will display::

    Relative path is "a/b"
    Relative path is ""

.. _GET_PARENT_PATH:

.. code-block:: cmake

  cmake_path(GET <path-var> PARENT_PATH <out-var>)

Returns the path to the parent directory.

If `HAS_RELATIVE_PATH`_ sub-command returns false, the result is a copy of
``<path-var>``. Otherwise, the result is ``<path-var>`` with one fewer element.

For Example:

  .. code-block:: cmake

    set (path "c:/a/b")
    cmake_path (GET path PARENT_PATH result)
    message ("Parent path is \"${result}\"")

    set (path "c:/")
    cmake_path (GET path PARENT_PATH result)
    message ("Parent path is \"${result}\"")

  Will display::

    Parent path is "c:/a"
    Relative path is "c:/"

Modification
^^^^^^^^^^^^

.. _APPEND:

.. code-block:: cmake

    cmake_path(APPEND <path-var> [<input>...] [OUTPUT_VARIABLE <out-var>])

Append all the ``<input>`` arguments to the ``<path-var>`` using ``/`` as
``directory-separator``.

For each ``<input>`` argument, the following algorithm (pseudo-code) applies:

  .. code-block:: cmake

    # <path> is the contents of <path-var>

    IF (<input>.is_absolute() OR
         (<input>.has_root_name() AND
          NOT <input>.root_name() STREQUAL <path>.root_name()))
      replaces <path> with <input>
      RETURN()
    ENDIF()

    IF (<input>.has_root_directory())
      remove any root-directory and the entire relative path from <path>
    ELSEIF (<path>.has_filename() OR
             (NOT <path-var>.has_root_directory() OR <path>.is_absolute()))
      appends directory-separator to <path>
    ENDIF()

    appends <input> omitting any root-name to <path>

.. _CONCAT:

.. code-block:: cmake

    cmake_path(CONCAT <path-var> [<input>...] [OUTPUT_VARIABLE <out-var>])

Concatenates all the ``<input>`` arguments to the ``<path-var>`` without
``directory-separator``.

.. _REMOVE_FILENAME:

.. code-block:: cmake

    cmake_path(REMOVE_FILENAME <path-var> [OUTPUT_VARIABLE <out-var>])

Removes the :ref:`filename <FILENAME_DEF>` component (as returned by
:ref:`GET ... FILENAME <GET_FILENAME>`) from ``<path-var>``.

After this function returns, if change is done in-place, `HAS_FILENAME`_
returns false for ``<path-var>``.

For Example:

  .. code-block:: cmake

    set (path "/a/b")
    cmake_path (REMOVE_FILENAME path)
    message ("First path is \"${path}\"")

    cmake_path (REMOVE_FILENAME path)
    message ("Second path is \"${result}\"")

  Will display::

    First path is "/a/"
    Second path is "/a/"

.. _REPLACE_FILENAME:

.. code-block:: cmake

    cmake_path(REPLACE_FILENAME <path-var> <input> [OUTPUT_VARIABLE <out-var>])

Replaces the :ref:`filename <FILENAME_DEF>` component from ``<path-var>`` with
``<input>``.

If ``<path-var>`` has no filename component (`HAS_FILENAME`_ returns false),
the path is unchanged.

Equivalent to the following:

  .. code-block:: cmake

    cmake_path(HAS_FILENAME path has_filename)
    if (has_filename)
      cmake_path(REMOVE_FILENAME path)
      cmake_path(APPEND path "replacement");
    endif()

.. _REMOVE_EXTENSION:

.. code-block:: cmake

    cmake_path(REMOVE_EXTENSION <path-var> [LAST_ONLY]
                                           [OUTPUT_VARIABLE <out-var>])

Removes the :ref:`extension <EXTENSION_DEF>`, if any, from ``<path-var>``.

.. _REPLACE_EXTENSION:

.. code-block:: cmake

    cmake_path(REPLACE_EXTENSION <path-var> [LAST_ONLY] <input>
                                 [OUTPUT_VARIABLE <out-var>])

Replaces the :ref:`extension <EXTENSION_DEF>` with ``<input>``.

  1. If ``<path-var>`` has an :ref:`extension <EXTENSION_DEF>`
     (`HAS_EXTENSION`_ is true), it is removed.
  2. A ``dot`` character is appended to ``<path-var>``, if ``<input>`` is not
     empty or does not begin with a ``dot`` character.
  3. ``<input>`` is appended as if `CONCAT`_ was used.


Equivalent to the following:

  .. code-block:: cmake

    cmake_path(REMOVE_EXTENSION path)
    if (NOT "input" MATCHES "^\\.")
      cmake_path(CONCAT path ".")
    endif()
    cmake_path(CONCAT path "input");

Generation
^^^^^^^^^^

.. _NORMAL_PATH:

.. code-block:: cmake

    cmake_path(NORMAL_PATH <path-var> [OUTPUT_VARIABLE <out-var>])

Normalize ``<path-var>``.

A path can be normalized by following this algorithm:

  1. If the path is empty, stop (normal form of an empty path is an empty
     path).
  2. Replace each ``directory-separator`` (which may consist of multiple
     separators) with a single ``/``.
  3. Replace each ``directory-separator`` character in the ``root-name`` with
     ``/``.
  4. Remove each ``dot`` and any immediately following ``directory-separator``.
  5. Remove each non-dot-dot filename immediately followed by a
     ``directory-separator`` and a ``dot-dot``, along with any immediately
     following ``directory-separator``.
  6. If there is ``root-directory``, remove all ``dot-dots`` and any
     ``directory-separators`` immediately following them.
  7. If the last filename is ``dot-dot``, remove any trailing
     ``directory-separator``.
  8. If the path is empty, add a ``dot`` (normal form of ``./`` is ``.``).

.. _cmake_path-RELATIVE_PATH:
.. _RELATIVE_PATH:

.. code-block:: cmake

    cmake_path(RELATIVE_PATH <path-var> [BASE_DIRECTORY <input>]
                                        [OUTPUT_VARIABLE <out-var>])

Returns ``<path-var>`` made relative to ``BASE_DIRECTORY`` argument. If
``BASE_DIRECTORY`` is not specified, the default base directory will be
:variable:`CMAKE_CURRENT_SOURCE_DIR`.

For reference, the algorithm used to compute the relative path is described
`here <https://en.cppreference.com/w/cpp/filesystem/path/lexically_normal>`_.

.. _PROXIMATE_PATH:

.. code-block:: cmake

    cmake_path(PROXIMATE_PATH <path-var> [BASE_DIRECTORY <input>]
                                         [OUTPUT_VARIABLE <out-var>])

If the value of `RELATIVE_PATH`_ is not an empty path, return
it. Otherwise return ``<path-var>``.

If ``BASE_DIRECTORY`` is not specified, the default base directory will be
:variable:`CMAKE_CURRENT_SOURCE_DIR`.

.. _ABSOLUTE_PATH:

.. code-block:: cmake

    cmake_path(ABSOLUTE_PATH <path-var> [BASE_DIRECTORY <input>] [NORMALIZE]
                                        [OUTPUT_VARIABLE <out-var>])

If ``<path-var>`` is a relative path (`IS_RELATIVE`_ is true), it is evaluated
relative to the given base directory specified by ``BASE_DIRECTORY`` option.

If ``BASE_DIRECTORY`` is not specifired, the default base directory will be
:variable:`CMAKE_CURRENT_SOURCE_DIR`.

When ``NORMALIZE`` option is specified, the path is :ref:`normalized
<NORMAL_PATH>` after the path computation.

Because ``cmake_path`` does not access to the filesystem, symbolic links are
not resolved. To compute a real path, use :command:`file(REAL_PATH)`
command.

Conversion
^^^^^^^^^^

.. _cmake_path-CMAKE_PATH:
.. _CMAKE_PATH:

.. code-block:: cmake

    cmake_path(CMAKE_PATH <path-var> [NORMALIZE] <input>)

Converts a native ``<input>`` path into cmake-style path with forward-slashes
(``/``). On Windows, the long filename marker is taken into account.

When ``NORMALIZE`` option is specified, the path is :ref:`normalized
<NORMAL_PATH>` before the conversion.

For Example:

  .. code-block:: cmake

    set (native_path "c:\\a\\b/..\\c")
    cmake_path (CMAKE_PATH path "${native_path}")
    message ("CMake path is \"${path}\"")

    cmake_path (CMAKE_PATH path NORMALIZE "${native_path}")
    message ("Normalized CMake path is \"${path}\"")

  Will display::

    CMake path is "c:/a/b/../c"
    Normalized CMake path is "c:/a/c"

.. _cmake_path-NATIVE_PATH:
.. _NATIVE_PATH:

.. code-block:: cmake

    cmake_path(NATIVE_PATH <path-var> [NORMALIZE] <out-var>)

Converts a cmake-style ``<path-var>`` into a native
path with platform-specific slashes (``\`` on Windows and ``/`` elsewhere).

When ``NORMALIZE`` option is specified, the path is :ref:`normalized
<NORMAL_PATH>` before the conversion.

.. _CONVERT:
.. _cmake_path-TO_CMAKE_PATH_LIST:
.. _TO_CMAKE_PATH_LIST:

.. code-block:: cmake

   cmake_path(CONVERT <input> TO_CMAKE_PATH_LIST <out-var> [NORMALIZE])

Converts a native ``<input>`` path into cmake-style path with forward-slashes
(``/``).  On Windows, the long filename marker is taken into account. The input can
be a single path or a system search path like ``$ENV{PATH}``.  A search path
will be converted to a cmake-style list separated by ``;`` characters. The
result of the conversion is stored in the ``<out-var>`` variable.

When ``NORMALIZE`` option is specified, the path is :ref:`normalized
<NORMAL_PATH>` before the conversion.

.. _cmake_path-TO_NATIVE_PATH_LIST:
.. _TO_NATIVE_PATH_LIST:

.. code-block:: cmake

  cmake_path(CONVERT <input> TO_NATIVE_PATH_LIST <out-var> [NORMALIZE])

Converts a cmake-style ``<input>`` path into a native path with
platform-specific slashes (``\`` on Windows and ``/`` elsewhere). The input can
be a single path or a cmake-style list. A list will be converted into a native
search path. The result of the conversion is stored in the ``<out-var>``
variable.

When ``NORMALIZE`` option is specified, the path is :ref:`normalized
<NORMAL_PATH>` before the conversion.

For Example:

  .. code-block:: cmake

    set (paths "/a/b/c" "/x/y/z")
    cmake_path (CONVERT "${paths}" TO_NATIVE_PATH_LIST native_paths)
    message ("Native path list is \"${native_paths}\"")

  Will display, on Windows::

    Native path list is "\a\b\c;\x\y\z"

  And on the all other systems::

    Native path list is "/a/b/c:/x/y/z"

Comparison
^^^^^^^^^^

.. _COMPARE:

.. code-block:: cmake

    cmake_path(COMPARE <path-var> EQUAL <input> <out-var>)
    cmake_path(COMPARE <path-var> NOT_EQUAL <input> <out-var>)

Compares the lexical representations of the path and another path.

For testing equality, the following algorithm (pseudo-code) apply:

  .. code-block:: cmake

    # <path> is the contents of <path-var>

    IF (NOT <path>.root_name() STREQUAL <input>.root_name())
      returns FALSE
    ELSEIF (<path>.has_root_directory() XOR <input>.has_root_directory())
      returns FALSE
    ENDIF()

    returns TRUE or FALSE if the relative portion of <path> is
      lexicographically equal or not to the relative portion of <input>.
      Comparison is performed path component-wise

Query
^^^^^

.. _HAS_ROOT_NAME:

.. code-block:: cmake

    cmake_path(HAS_ROOT_NAME <path-var> <out-var>)

Checks if ``<path-var>`` has ``root-name``.

.. _HAS_ROOT_DIRECTORY:

.. code-block:: cmake

    cmake_path(HAS_ROOT_DIRECTORY <path-var> <out-var>)

Checks if ``<path-var>`` has ``root-directory``.

.. _HAS_ROOT_PATH:

.. code-block:: cmake

    cmake_path(HAS_ROOT_PATH <path-var> <out-var>)

Checks if ``<path-var>`` has root path.

Effectively, checks if ``<path-var>`` has ``root-name`` and ``root-directory``.

.. _HAS_FILENAME:

.. code-block:: cmake

    cmake_path(HAS_FILENAME <path-var> <out-var>)

Checks if ``<path-var>`` has a :ref:`filename <FILENAME_DEF>`.

.. _HAS_EXTENSION:

.. code-block:: cmake

    cmake_path(HAS_EXTENSION <path-var> <out-var>)

Checks if ``<path-var>`` has an :ref:`extension <EXTENSION_DEF>`. If the first
character in the filename is a period, it is not treated as an extension (for
example ".profile").

.. _HAS_STEM:

.. code-block:: cmake

    cmake_path(HAS_STEM <path-var> <out-var>)

Checks if ``<path-var>`` has stem (:ref:`GET ... STEM <GET_STEM>` returns a non
empty path).

.. _HAS_RELATIVE_PATH:

.. code-block:: cmake

    cmake_path(HAS_RELATIVE_PATH <path-var> <out-var>)

Checks if ``<path-var>`` has relative path (`GET_RELATIVE_PATH`_ returns a
non-empty path).

.. _HAS_PARENT_PATH:

.. code-block:: cmake

    cmake_path(HAS_PARENT_PATH <path-var> <out-var>)

Checks if ``<path-var>`` has parent path. The result is true except if the path
is only composed of a :ref:`filename <FILENAME_DEF>`.

.. _IS_ABSOLUTE:

.. code-block:: cmake

    cmake_path(IS_ABSOLUTE <path-var> <out-var>)

Checks if ``<path-var>`` is absolute.

An absolute path is a path that unambiguously identifies the location of a file
without reference to an additional starting location.

.. _IS_RELATIVE:

.. code-block:: cmake

    cmake_path(IS_RELATIVE <path-var> <out-var>)

Checks if path is relative (i.e. not :ref:`absolute <IS_ABSOLUTE>`).

.. _IS_PREFIX:

.. code-block:: cmake

    cmake_path(IS_PREFIX <path-var> <input> [NORMALIZE] <out-var>)

Checks if ``<path-var>`` is the prefix of ``<input>``.

When ``NORMALIZE`` option is specified, the paths are :ref:`normalized
<NORMAL_PATH>` before the check.

Hashing
^^^^^^^

.. _HASH:

.. code-block:: cmake

    cmake_path(HASH <path-var> [NORMALIZE] <out-var>)

Compute hash value of ``<path-var>`` such that if for two paths (``p1`` and
``p2``) are equal (:ref:`COMPARE ... EQUAL <COMPARE>`) then hash value of p1 is
equal to hash value of p2.

When ``NORMALIZE`` option is specified, the paths are :ref:`normalized
<NORMAL_PATH>` before the check.
