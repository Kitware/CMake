  ``CMAKE_REQUIRED_FLAGS``
    String of additional flags to pass to the compiler. The string must be
    space-delimited--a :ref:`;-list <CMake Language Lists>` will not work.
    The contents of :variable:`CMAKE_<LANG>_FLAGS <CMAKE_<LANG>_FLAGS>` and
    its associated configuration-specific variable are automatically added
    to the compiler command before the contents of ``CMAKE_REQUIRED_FLAGS``.
