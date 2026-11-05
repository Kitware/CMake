CTEST_SCRIPT_DIRECTORY
----------------------

The directory component of the path to the top-level CTest script specified by
the :option:`ctest -S` option.  It is given as it appears in that path, so it
may be relative.

The file name part is available in :variable:`CTEST_SCRIPT_NAME`.  The concept
is similar to :variable:`CMAKE_CURRENT_LIST_DIR`.
