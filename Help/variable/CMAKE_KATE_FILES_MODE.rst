CMAKE_KATE_FILES_MODE
---------------------

.. versionadded:: 3.27

This cache variable is used by the Kate project generator and controls
to what mode the ``files`` entry in the project file will be set.  See
:manual:`cmake-generators(7)`.

Possible values are ``AUTO``, ``SVN``, ``GIT`` and ``LIST``.

When set to ``LIST``, CMake will put the list of source files known to CMake
in the project file.
When set to ``SVN``, CMake will put ``svn`` into the project file so that Kate
will use svn to retrieve the list of files in the project.
When set to ``GIT``, CMake will put ``git`` into the project file so that Kate
will use git to retrieve the list of files in the project.
When unset or set to ``AUTO``, CMake will try to detect whether the
source directory is part of a git or svn checkout or not, and put the
respective entry into the project file.
