CMD_CPS_NON_CONFORMING_LOCATION
-------------------------------

.. versionadded:: 4.6

.. diagnostic::
  :default: ignore
  :parent: CMD_AUTHOR

  Warn if :command:`find_package` finds a CPS package from a location that does
  not conform to the CPS specification for `Package Searching`_.  This may
  happen because CMake searches for packages in more locations than are
  officially specified by CPS.  If the severity level is ``SEND_ERROR`` or
  higher, CMake will not cache the package location and subsequent configure
  attempts will also fail.  Otherwise, the diagnostic is only emitted when CMake
  initially discovers the package.

.. _`Package Searching`: https://cps-org.github.io/cps/searching.html
