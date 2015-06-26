WINDOWS_EXPORT_ALL_SYMBOLS
--------------------------

This property is implemented only for MS-compatible tools on Windows.

Enable this boolean property to automatically create a module definition
(``.def``) file with all global symbols found in the input ``.obj`` files
for a ``SHARED`` library on Windows.  The module definition file will be
passed to the linker causing all symbols to be exported from the ``.dll``.
For global *data* symbols, ``__declspec(dllimport)`` must still be used when
compiling against the code in the ``.dll``.  All other function symbols will
be automatically exported and imported by callers.  This simplifies porting
projects to Windows by reducing the need for explicit ``dllexport`` markup,
even in ``C++`` classes.

This property is initialized by the value of
the :variable:`CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS` variable if it is set
when a target is created.
