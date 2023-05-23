ep-update-disconnected
----------------------

* The ``update`` and ``patch`` steps of an :module:`ExternalProject` will now
  always re-execute if any of their details change, even if
  ``UPDATE_DISCONNECTED`` was set to true in the call to
  :command:`ExternalProject_Add`. If using the GIT download method and the
  ``GIT_TAG`` is changed and the new ``GIT_TAG`` isn't already known locally,
  this is now a fatal error instead of silently using the previous ``GIT_TAG``.

* When ``UPDATE_DISCONNECTED`` is set to true in a call to
  :command:`ExternalProject_Add`, the ``configure`` step will no longer
  re-run on every build. It will only re-run if details of the ``download``,
  ``update`` or ``patch`` step change.
