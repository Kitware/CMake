CMAKE_VERIFY_INTERFACE_HEADER_SETS
----------------------------------

.. versionadded:: 3.24

This variable is used to initialize the
:prop_tgt:`VERIFY_INTERFACE_HEADER_SETS` property of targets when they are
created.  Setting it to true enables header set verification.

Projects should not set this variable, it is intended as a developer
control to be set on the :manual:`cmake(1)` command line or other
equivalent methods.  The developer must have the ability to enable or
disable header set verification according to the capabilities of their own
machine and compiler.

By default, this variable is not set, which will result in header set
verification being disabled.
