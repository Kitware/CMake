SOURCE_SET_<NAME>
-----------------

.. versionadded:: 4.4

Semicolon-separated list of files in the target's ``<NAME>`` source set,
which has the set type ``SOURCES``. If any of the paths are relative,
they are computed relative to the target's source directory. The property
supports :manual:`generator expressions <cmake-generator-expressions(7)>`.

This property is normally only set by :command:`target_sources(FILE_SET)`
rather than being manipulated directly.

See :prop_tgt:`SOURCE_SET` for the list of files in the default source set.
See :prop_tgt:`SOURCE_SETS` for the file set names of all source sets.
