CMAKE_INSTALL_REMOVE_ENVIRONMENT_RPATH
--------------------------------------

Removes compiler defined rpaths durimg installation.

``CMAKE_INSTALL_REMOVE_ENVIRONMENT_RPATH`` is a boolean that if set to ``true``
removes compiler defined rpaths from the project if the user also defines rpath
with :prop_tgt:`INSTALL_RPATH`. This is used to initialize the target property
:prop_tgt:`INSTALL_REMOVE_ENVIRONMENT_RPATH` for all targets.
