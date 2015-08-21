CMAKE_ERROR_DEPRECATED
----------------------

Whether to issue deprecation errors for macros and functions.

If ``TRUE``, this can be used by macros and functions to issue fatal
errors when deprecated macros or functions are used.  This variable is
``FALSE`` by default.

These errors can be enabled with the ``-Werror=deprecated`` option, or
disabled with the ``-Wno-error=deprecated`` option, when running
:manual:`cmake(1)`.
