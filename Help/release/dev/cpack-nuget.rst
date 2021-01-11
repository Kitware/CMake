cpack-nuget
-----------

* The :cpack_gen:`CPack NuGet Generator` gained options:

  - :variable:`CPACK_NUGET_PACKAGE_ICON` and
    :variable:`CPACK_NUGET_<compName>_PACKAGE_ICON`
    allow package icons to be specified by local files.
  - :variable:`CPACK_NUGET_PACKAGE_LICENSE_EXPRESSION` and
    :variable:`CPACK_NUGET_<compName>_PACKAGE_LICENSE_EXPRESSION` add
    support for specifying licenses recognized by the
    `Software Package Data Exchange`_ (SPDX).
  - :variable:`CPACK_NUGET_PACKAGE_LICENSE_FILE_NAME` and
    :variable:`CPACK_NUGET_<compName>_PACKAGE_LICENSE_FILE_NAME` allow
    licenses to be specified by local files.
  - :variable:`CPACK_NUGET_PACKAGE_LANGUAGE` and
    :variable:`CPACK_NUGET_<compName>_PACKAGE_LANGUAGE` allow the locale
    for a package to be specified, for example ``en_CA``.

 Some other variables have been deprecated to reflect changes in the
 NuGet specification:

 - :variable:`CPACK_NUGET_PACKAGE_ICONURL` and
   :variable:`CPACK_NUGET_<compName>_PACKAGE_ICONURL` have been deprecated;
   replace with a reference to a local icon file.
 - :variable:`CPACK_NUGET_PACKAGE_LICENSEURL` and
   :variable:`CPACK_NUGET_<compName>_PACKAGE_LICENSEURL` have been deprecated;
   replace with a reference to the project's license file or SPDX
   license expression.

.. _Software Package Data Exchange: https://spdx.org/
