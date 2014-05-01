ExternalData-missing-not-fatal
------------------------------

* The :module:`ExternalData` module learned to tolerate a ``DATA{}``
  reference to a missing source file with a warning instead of
  rejecting it with an error.  This helps developers write new
  ``DATA{}`` references to test reference outputs that have not
  yet been created.
