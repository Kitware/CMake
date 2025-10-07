define-msvc-char-set
--------------------

* For builds targeting the MSVC ABI, all generators now add the ``_MBCS``
  preprocessor definition when compiling sources unless ``_UNICODE`` or ``_SBCS``
  is found. See policy :policy:`CMP0204`.
