cmake-W-options
---------------

* The :variable:`CMAKE_ERROR_DEPRECATED` variable can now be set using the
  ``-Werror=deprecated`` and ``-Wno-error=deprecated`` :manual:`cmake(1)`
  options.

* The :variable:`CMAKE_WARN_DEPRECATED` variable can now be set using the
  ``-Wdeprecated`` and ``-Wno-deprecated`` :manual:`cmake(1)` options.

* :manual:`cmake(1)` gained options ``-Werror=dev`` and ``-Wno-error=dev``
  to control whether developer warnings intended for project authors
  are treated as errors.
