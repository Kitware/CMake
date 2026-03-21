CMP0213
-------

.. versionadded:: 4.4

:command:`file(ARCHIVE_CREATE)` and :command:`file(ARCHIVE_EXTRACT)`
encode archive paths as UTF-8 by default.

CMake 4.3 and below always used the ``OEM`` (current locale) encoding for archive
paths.  CMake 4.4 offer the ``ENCODING`` option, but prefer to use ``UTF-8``
by default.  This policy provides compatibility for projects that have not
been updated to specify the encoding or accept the new default.

The ``OLD`` behavior for this policy is to use the current locale as the
default encoding, which is also known as an ``OEM`` encoding.
The ``NEW`` behavior uses ``UTF-8`` as the default encoding.

.. |INTRODUCED_IN_CMAKE_VERSION| replace:: 4.4
.. |WARNS_OR_DOES_NOT_WARN| replace:: does *not* warn
.. include:: include/STANDARD_ADVICE.rst

.. include:: include/DEPRECATED.rst
