VERBATIM
--------

.. versionadded:: 4.5

``VERBATIM`` is a boolean indicating that all arguments to the commands
produced by the instantiation of the rule will be escaped properly for the
build tool so that the invoked command receives each argument unchanged.  Note
that one level of escapes is still used by the CMake language processor before
:command:`add_custom_command` command even sees the arguments.

By default, ``VERBATIM`` has a true value because it is recommended as it
enables correct behavior.  When ``VERBATIM`` is not given the behavior is
platform specific because there is no protection of tool-specific special
characters.
