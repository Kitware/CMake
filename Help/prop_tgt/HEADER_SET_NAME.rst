HEADER_SET_<NAME>
-----------------

.. versionadded:: 3.23

Semicolon-separated list of headers in the named header set ``<NAME>`` created
by :command:`target_sources(FILE_SET)`. This property supports
:manual:`generator expressions <cmake-generator-expressions(7)>`. If any of the
headers are relative paths, they are computed relative to the target's source
directory.
