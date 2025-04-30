GNUInstallDirs-special-cases
----------------------------

* The :module:`GNUInstallDirs` module now prefers to default
  ``SYSCONFDIR``, ``LOCALSTATEDIR``, and ``RUNSTATEDIR`` to
  absolute paths when installing to special prefixes.
  See policy :policy:`CMP0192`.

* The :module:`GNUInstallDirs` module now caches ``CMAKE_INSTALL_*``
  variables with their leading ``usr/`` for install prefix ``/``.
  See policy :policy:`CMP0193`.
