update-curl
-----------

* The version of curl bundled with CMake no longer accepts URLs of the form
  ``file://c:/...`` on Windows due to a change in upstream curl 7.52.  Use
  the form ``file:///c:/...`` instead to work on all versions.
