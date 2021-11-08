HEADER_SET
----------

.. versionadded:: 3.23

Semicolon-separated list of headers in the default header set created by
:command:`target_sources(FILE_SET)`. This property supports
:manual:`generator expressions <cmake-generator-expressions(7)>`. If any of the
headers are relative paths, they are computed relative to the target's source
directory.
