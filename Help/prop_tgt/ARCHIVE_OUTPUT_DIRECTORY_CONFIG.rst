ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>
---------------------------------

Per-configuration output directory for ARCHIVE target files.

This is a per-configuration version of ARCHIVE_OUTPUT_DIRECTORY, but
multi-configuration generators (VS, Xcode) do NOT append a
per-configuration subdirectory to the specified directory.  This
property is initialized by the value of the variable
CMAKE_ARCHIVE_OUTPUT_DIRECTORY_<CONFIG> if it is set when a target is
created.
