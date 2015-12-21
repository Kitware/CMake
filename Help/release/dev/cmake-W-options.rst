cmake-W-options
---------------

* The :variable:`CMAKE_WARN_DEPRECATED` variable can now be set using the
  ``-Wdeprecated`` and ``-Wno-deprecated`` :manual:`cmake(1)` options.

* The ``-Wdev`` and ``-Wno-dev`` :manual:`cmake(1)` options now also enable
  and suppress the deprecated warnings output by default.

* Warnings about deprecated functionality are now enabled by default.
  They may be suppressed with ``-Wno-deprecated`` or by setting the
  :variable:`CMAKE_WARN_DEPRECATED` variable to false.

* Warnings about deprecated functionality can now be controlled in the
  :manual:`cmake-gui(1)` application.

* The suppression of developer warnings as errors can now be controlled with
  the new ``-Werror=dev`` and ``-Wno-error=dev`` :manual:`cmake(1)` options.

* The :variable:`CMAKE_ERROR_DEPRECATED` variable can now be set using the
  ``-Werror=deprecated`` and ``-Wno-error=deprecated`` :manual:`cmake(1)`
  options.
