include_directories
-------------------

Add include directories to the build.

::

  include_directories([AFTER|BEFORE] [SYSTEM] dir1 dir2 ...)

Add the given directories to those the compiler uses to search for
include files.  Relative paths are interpreted as relative to the
current source directory.

The include directories are added to the directory property
INCLUDE_DIRECTORIES for the current CMakeLists file.  They are also
added to the target property INCLUDE_DIRECTORIES for each target in
the current CMakeLists file.  The target property values are the ones
used by the generators.

By default the directories are appended onto the current list of
directories.  This default behavior can be changed by setting
CMAKE_INCLUDE_DIRECTORIES_BEFORE to ON.  By using AFTER or BEFORE
explicitly, you can select between appending and prepending,
independent of the default.

If the SYSTEM option is given, the compiler will be told the
directories are meant as system include directories on some platforms
(signalling this setting might achieve effects such as the compiler
skipping warnings, or these fixed-install system files not being
considered in dependency calculations - see compiler docs).
