cpack-deb-imporvements
----------------------

* The :module:`CPackDeb` module learned how to generate ``DEBIAN/shlibs``
  contorl file when package contains shared libraries.

* The :module:`CPackDeb` module learned how to generate ``DEBIAN/postinst`` and
  ``DEBIAN/postrm`` files if the package installs libraries in
  ldconfig-controlled locations (e.g. ``/lib/``, ``/usr/lib/``).

* The :module:`CPackDeb` module learned how to generate dependencies between
  Debian packages if multi-component setup is used and
  :variable:`CPACK_COMPONENT_<compName>_DEPENDS` variables are set.
  For backward compatibility this feature is disabled by default. See
  :variable:`CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS`.

* The :module:`CPackDeb` module learned how to set the package release number
  (``DebianRevisionNumber`` in package file name).  See
  :variable:`CPACK_DEBIAN_PACKAGE_RELEASE`.

* The :module:`CPackDeb` module now generates properly-named Debian packages::

    <PackageName>_<VersionNumber>-<DebianRevisionNumber>_<DebianArchitecture>.deb

  This breaks compatibility with previous versions.

* The :module:`CPackDeb` module learned how to set the package architecture
  per-component.  See :variable:`CPACK_DEBIAN_<COMPONENT>_PACKAGE_ARCHITECTURE`.
