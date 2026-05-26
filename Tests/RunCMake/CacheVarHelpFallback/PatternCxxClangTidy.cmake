# Defines a placeholder-shaped built-in cache variable on the command
# line with no ``:TYPE=`` annotation.  No exact-match entry exists for
# ``CMAKE_CXX_CLANG_TIDY`` in ``cmCacheDocumentationTable``;
# ``cmake::ProcessCacheArg`` therefore consults the pattern table
# (``cmCachePatternTable``), matches the ``CMAKE_<LANG>_CLANG_TIDY``
# pattern, and installs the pattern's Summary as the entry's HELPSTRING.
# Type and value are preserved verbatim.  The project listfile itself
# does not ``set(...)`` the variable; the cache entry materializes
# solely through ``-D``.
