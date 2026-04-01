SOURCE_DIRS_<NAME>
------------------

.. versionadded:: 4.4

Semicolon-separated list of base directories of the target's ``<NAME>``
source set, which has the set type ``SOURCES``. The property supports
:manual:`generator expressions <cmake-generator-expressions(7)>`.

This property is normally only set by :command:`target_sources(FILE_SET)`
rather than being manipulated directly.

See :prop_tgt:`SOURCE_DIRS` for the list of base directories in the
default source set. See :prop_tgt:`SOURCE_SETS` for the file set names of all
source sets.
