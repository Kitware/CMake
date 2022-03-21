LINK_LIBRARY-WHOLE_ARCHIVE
--------------------------

* The :genex:`LINK_LIBRARY` generator expression gained the feature
  ``WHOLE_ARCHIVE`` to force load of all members in a static library. This
  feature is supported on the following target platforms:

  * all ``Apple`` variants
  * ``Linux``
  * all ``BSD`` variants
  * ``SunOS``
  * ``Windows``
  * ``CYGWIN``
  * ``MSYS``
