SOURCE_SET
----------

.. versionadded:: 4.4

Semicolon-separated list of files in the target's default source set,
(i.e. the file set with name and type ``SOURCES``). If any of the paths
are relative, they are computed relative to the target's source directory.
The property supports
:manual:`generator expressions <cmake-generator-expressions(7)>`.

This property is normally only set by :command:`target_sources(FILE_SET)`
rather than being manipulated directly.

See :prop_tgt:`SOURCE_SET_<NAME>` for the list of files in other source sets.
