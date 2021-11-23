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

``COPY``, empty or unset
  Duplicate the file at its destination.  This is the default behavior.

``ABS_SYMLINK``
  Create an *absolute* symbolic link to the source file at the destination.
  Halt with an error if the link cannot be created.

``ABS_SYMLINK_OR_COPY``
  Like ``ABS_SYMLINK`` but fall back to silently copying if the symlink
  couldn't be created.

``REL_SYMLINK``
  Create a *relative* symbolic link to the source file at the destination.
  Halt with an error if the link cannot be created.

``REL_SYMLINK_OR_COPY``
  Like ``REL_SYMLINK`` but fall back to silently copying if the symlink
  couldn't be created.

``SYMLINK``
  Try as if through ``REL_SYMLINK`` and fall back to ``ABS_SYMLINK`` if the
  referenced file cannot be expressed using a relative path.
  Halt with an error if the link cannot be created.

``SYMLINK_OR_COPY``
  Like ``SYMLINK`` but fall back to silently copying if the symlink couldn't
  be created.

Installing symbolic links rather than copying files can help conserve storage space because files do
not have to be duplicated on disk. However, modifications applied to the source immediately affects
the symbolic link and vice versa. *Use with caution*.

.. note:: ``CMAKE_INSTALL_MODE`` only affects files, *not* directories.

.. note:: Symbolic links are not available on all platforms.
