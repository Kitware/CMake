fileapi-provide-glob-dependent
------------------------------

* The :manual:`cmake-file-api(7)` "cmakeFiles" version 1 object's ``version``
  field has been updated to 1.1.

* The :manual:`cmake-file-api(7)` "cmakeFiles" version 1 object gained a
  ``globsDependent`` field to report :command:`file(GLOB)` calls using
  ``CONFIGURE_DEPENDS``.
