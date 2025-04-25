externalproject-archive-types
-----------------------------

* The :module:`ExternalProject` module no longer checks the ``URL`` archive
  file extension.  Any archive type that :option:`cmake -E tar <cmake-E tar>`
  can extract is now allowed.
