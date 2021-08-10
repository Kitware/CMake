CMAKE_INSTALL_MODE
------------------

.. versionadded:: 3.22

.. include:: ENV_VAR.txt

The ``CMAKE_INSTALL_MODE`` environment variable allows users to operate
CMake in an alternate mode of :command:`file(INSTALL)` and :command:`install()`.

The default behavior for an installation is to copy a source file from a
source directory into a destination directory. This environment variable
however allows the user to override this behavior, causing CMake to create
symbolic links instead.

.. note::
  A symbolic link consists of a reference file path rather than contents of its own,
  hence there are two ways to express the relation, either by a relative or an absolute path.

The following values are allowed for ``CMAKE_INSTALL_MODE``:

* empty, unset or ``COPY``: default behavior, duplicate the file at its destination
* ``ABS_SYMLINK``: create an *absolute* symbolic link to the source file at the destination *or fail*
* ``ABS_SYMLINK_OR_COPY``: like ``ABS_SYMLINK`` but silently copy on error
* ``REL_SYMLINK``: create an *relative* symbolic link to the source file at the destination *or fail*
* ``REL_SYMLINK_OR_COPY``: like ``REL_SYMLINK`` but silently copy on error
* ``SYMLINK``: try as if through ``REL_SYMLINK`` and fall back to ``ABS_SYMLINK`` if the referenced
  file cannot be expressed using a relative path. Fail on error.
* ``SYMLINK_OR_COPY``: like ``SYMLINK`` but silently copy on error

Installing symbolic links rather than copying files can help conserve storage space because files do
not have to be duplicated on disk. However, modifications applied to the source immediately affects
the symbolic link and vice versa. *Use with caution*.

.. note:: ``CMAKE_INSTALL_MODE`` only affects files, *not* directories.

.. note:: Symbolic links are not available on all platforms.
