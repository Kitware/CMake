ExternalProject-no-extract-timestamp
------------------------------------

* The :command:`ExternalProject_Add` command gained a new
  ``DOWNLOAD_EXTRACT_TIMESTAMP`` option for controlling whether the timestamps
  of extracted contents are set to match those in the archive when the ``URL``
  download method is used. A new policy :policy:`CMP0135` was added to control
  the default behavior when the new option is not used.
