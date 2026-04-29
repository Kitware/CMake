write-basic-package-version-file-compatibility
----------------------------------------------

* The :module:`CMakePackageConfigHelpers` module's
  :command:`write_basic_package_version_file` command now supports
  ``SamePatchVersion``, ``SameFullVersion``, and ``SemanticVersion``
  compatibility modes. The older ``ExactVersion`` mode is now documented as
  deprecated in favor of the more explicit ``SamePatchVersion`` and
  ``SameFullVersion`` modes. The ``SemanticVersion`` mode treats ``0.y.z``
  versions more strictly than ``1.y.z`` and later versions.
