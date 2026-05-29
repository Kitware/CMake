# Defines a built-in cache variable on the command line with no
# ``:TYPE=`` annotation.  ``cmake::ProcessCacheArg`` must consult the
# built-in documentation table and install the table summary as the
# entry's HELPSTRING.  Type and value are preserved verbatim, so the
# entry remains ``:UNINITIALIZED=`` (consistent with the project-side
# behavior, which would re-type later in ``cmMakefile::AddCacheDefinition``
# only if a ``set(... CACHE BOOL ...)`` call appeared in a listfile).
# The project listfile itself does not ``set(...)`` the variable; the
# cache entry materializes solely through ``-D``.
