CMAKE_LINK_WHAT_YOU_USE_CHECK
-----------------------------

.. versionadded:: 3.22

Defines the command executed after the link step to check libraries usage.
This check is currently only defined on ``ELF`` platforms with value
``ldd -u -r``.

See also :variable:`CMAKE_<LANG>_LINK_WHAT_YOU_USE_FLAG` variables.
