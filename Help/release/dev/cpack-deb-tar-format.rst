cpack-deb-tar-format
--------------------

* The :module:`CPack` module no longer defaults to the ``paxr`` value in the
  :variable:`CPACK_DEBIAN_ARCHIVE_TYPE` variable, because ``dpkg`` has
  never supported the PAX tar format. The ``paxr`` value will be mapped
  to ``gnutar`` and a deprecation message emitted.
