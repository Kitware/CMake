CMAKE_WARN_DEPRECATED
---------------------

.. deprecated:: 4.4

Whether to issue warnings for deprecated functionality.

If not ``FALSE``, use of deprecated functionality will issue warnings.
If this variable is not set, CMake behaves as if it were set to ``TRUE``.

When running :manual:`cmake(1)`, this option can be enabled with the
:option:`-Wdeprecated <cmake -W>` option, or disabled with the
:option:`-Wno-deprecated <cmake -Wno->` option.
