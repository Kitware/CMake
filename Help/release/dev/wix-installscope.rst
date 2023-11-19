wix-installscope
----------------

* The :cpack_gen:`CPack WIX Generator` gained a new variable,
  :variable:`CPACK_WIX_INSTALL_SCOPE`, to control the
  ``InstallScope`` property of WiX MSI installers.

* The :cpack_gen:`CPack WIX Generator` now produces WiX MSI installers
  that create start menu and uninstall entries for all users by default,
  as documented by the :variable:`CPACK_WIX_INSTALL_SCOPE` variable
  ``perMachine`` value.  Previously, without a custom WiX template,
  it produced installers that would only create start menu and uninstall
  entries for the current user, even though they install for all users.
