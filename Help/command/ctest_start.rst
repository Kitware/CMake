ctest_start
-----------

Starts the testing for a given model

::

  ctest_start(Model [TRACK <track>] [APPEND] [source [binary]])

Starts the testing for a given model.  The command should be called
after the binary directory is initialized.  If the 'source' and
'binary' directory are not specified, it reads the
CTEST_SOURCE_DIRECTORY and CTEST_BINARY_DIRECTORY.  If the track is
specified, the submissions will go to the specified track.  If APPEND
is used, the existing TAG is used rather than creating a new one based
on the current time stamp.
