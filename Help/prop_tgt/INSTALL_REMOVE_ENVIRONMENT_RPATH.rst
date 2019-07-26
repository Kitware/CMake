INSTALL_REMOVE_ENVIRONMENT_RPATH
--------------------------------

Removes compiler defined rpaths durimg installation.

``INSTALL_REMOVE_ENVIRONMENT_RPATH`` is a boolean that if set to ``True`` will
remove compiler defined rpaths from the project if the user also defines rpath
with :prop_tgt:`INSTALL_RPATH`.  This property is initialized by whether the
value of :variable:`CMAKE_INSTALL_REMOVE_ENVIRONMENT_RPATH` is set when a
target is created.
