BASE_DIRS
---------

List of base directories of the file set. The :command:`target_sources` command
sets or adds to the ``BASE_DIRS`` file set property and is the usual way to
manipulate it.

Contents of ``BASE_DIRS`` may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.

Any relative paths are considered relative to the target's source directory. No
two base directories for a file set may be sub-directories of each other. This
requirement must be met across all base directories added to a file set.
