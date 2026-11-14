cpack-wix-perUserOrMachine
--------------------------

* The :cpack_gen:`CPack WIX Generator` gained a ``perUserOrMachine`` value
  for :variable:`CPACK_WIX_INSTALL_SCOPE` to create installers that can
  install for the current user or for all users.

* The :cpack_gen:`CPack WIX Generator` now creates the
  :variable:`CPACK_WIX_CMAKE_PACKAGE_REGISTRY` entry under
  ``HKEY_CURRENT_USER`` for ``perUser`` installers.  Previously they
  failed to install when it was set.

* The :cpack_gen:`CPack WIX Generator` now creates ``perUser`` installers
  that refuse to install for all users.
