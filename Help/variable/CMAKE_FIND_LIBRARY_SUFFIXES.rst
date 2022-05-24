CMAKE_FIND_LIBRARY_SUFFIXES
---------------------------

Suffixes to append when looking for libraries.

This specifies what suffixes to add to library names when the
:command:`find_library` command looks for libraries.  On Windows systems this
is typically ``.lib`` and, depending on the compiler, ``.dll.a``, ``.a``
(e.g. GCC and Clang), so when it tries to find the ``foo`` library, it will
look for ``[<prefix>]foo.lib`` and/or ``[<prefix>]foo[.dll].a``, depending on
the compiler used and the ``<prefix>`` specified in the
:variable:`CMAKE_FIND_LIBRARY_PREFIXES`.
